classdef DEFile < handle

    properties
        ptr (1,1) {mustBeA(ptr, 'lib.pointer')} = libpointer('voidPtrPtr')
        fname {mustBeTextScalar} = ''
        memory (1,1) {mustBeNumericOrLogical} = false
        readonly (1,1) {mustBeNumericOrLogical} = false 
        read_to_iris (1,1) {mustBeNumericOrLogical} = false
        iris_colnames_field {mustBeTextScalar} = ''
    end

    methods (Static)
        function de = DEFile(path, o)
            arguments
                path {mustBeTextScalar} = ''
                o.readonly = false
                o.memory = false
                o.truncate = false
                o.read_to_iris = false
                o.iris_colnames_field = ''
            end
            if isa(path, 'string')
                path = char(path)
            end
            ptr = libpointer('voidPtrPtr', 0);
            if o.memory
                DAEC.check_call('de_open_memory', ptr);
            elseif o.readonly
                DAEC.check_call('de_open_readonly', path, ptr);
            else
                DAEC.check_call('de_open', path, ptr);
                if o.truncate
                    DAEC.check_call('de_truncate', ptr);
                end
            end
            de.ptr = ptr;
            de.fname = path;
            de.memory = o.memory;
            de.readonly = o.readonly;
            de.read_to_iris = o.read_to_iris;
            de.iris_colnames_field = o.iris_colnames_field;
        end
    end

    methods % daec files

        function tf = isopen(de)
            tf = isvalid(de) && not(de.ptr.isNull);
        end

        function de = truncate(de)
            [~] = DAEC.check_call('de_truncate', de.ptr);
        end

        function de = close(de)
            [~] = DAEC.check_call('de_close', de.ptr);
            de.ptr = libpointer('voidPtrPtr');
        end

        function delete(de)
            close(de);
        end

    end

    methods % catalogs

        function id = find_object(de, pid, name)
            if nargin == 2
                name = pid;
                pid = 0;
            end
            [~, ~, id] = DAEC.check_call('de_find_object', de.ptr, pid, name, -1);
        end

        function attr = get_all_attributes(de, id, delim)
            if nargin < 3
                % If any attribute name or value contains the delimiter there 
                % will be trouble. Caller can override as necessary. 
                delim = '||';  % default delimiter. 
            end
            pn = lib.pointer('stringPtrPtr', {''}); % pointer to names
            pv = lib.pointer('stringPtrPtr', {''}); % pointer to values
            num = lib.pointer('int64Ptr', -1);
            DAEC.check_call('de_get_all_attributes', de.ptr, id, delim, num, pn, pv);
            if isempty(pn.Value{1})
                attr = struct();
                return
            end
            names = split(pn.Value{1}, delim);
            assert(numel(names) == num.Value, 'Inconsistent number of names')
            values = split(pv.Value{1}, delim);
            assert(numel(values) == num.Value, 'Inconsistent number of values')
            if all(cellfun(@isvarname, names), 'all')
                nv = [names(:) values(:)]';
                attr = struct(nv{:});
            else
                attr = containers.Map(names, values);
            end
        end

        function list = list_catalog(de, id)
            OK = DAEC.enums.status_t.DE_SUCCESS;
            DONE = DAEC.enums.status_t.DE_NO_OBJ;
            search = lib.pointer('voidPtrPtr', 0);
            DAEC.check_call('de_list_catalog', de.ptr, id, search);
            list = struct;
            num = 1;
            po = lib.pointer('object_tPtr', struct);
            try
                while true
                    status = DAEC.call('de_next_object', search, po);
                    if status == OK
                        list(num).id = po.Value.id;
                        list(num).pid = po.Value.pid;
                        list(num).obj_class = po.Value.obj_class;
                        list(num).obj_type = po.Value.obj_type;
                        list(num).name = po.Value.name;
                        num = num + 1;
                    elseif status == DONE
                        break
                    else
                        DAEC.check(status);
                    end
                end
            catch exception
                DAEC.check_call('de_finalize_search', search);
                rethrow(exception);
            end
            DAEC.check_call('de_finalize_search', search);
        end

        function id = new_catalog(de, pid, name)
            if nargin == 1
                pid = 0;
            end
            [~, ~, id] = DAEC.check_call('de_new_catalog', de.ptr, pid, name, -1);
        end

        function sz = catalog_size(de, pid)
            if nargin == 1
                pid = 0;
            end
            [~, sz] = DAEC.check_call('de_catalog_size', de.ptr, pid, -1);
        end

        function obj = read(de, pid, name)
            if nargin == 1
                obj = read_id(de, 0);
            else
                if nargin == 2
                    name = pid;
                    pid = 0;
                end
                
                id = find_object(de, pid, name);
                obj = read_id(de, id);
            end
        end

        function obj = read_id(de, id)
            if nargin == 1
                id = 0;
            end
            % de_load_object(voidPtr, int64, object_tPtr)
            objectStruct = libstruct('object_t');
            objectStruct.obj_type = int32(0);
            objectStruct.obj_class = int32(0);
            objectPtr = libpointer('object_t', objectStruct);
            [~, obj_t] = DAEC.check_call('de_load_object', de.ptr, id, objectPtr);
            % now get the value
            switch DAEC.enums.class_t.(obj_t.obj_class)
                case DAEC.enums.class_t.class_catalog
                    obj = struct();
                    fields = list_catalog(de, obj_t.id);
                    for i = 1:length(fields)
                        sub_obj = fields(i);
                        obj.(sub_obj.name) = read_id(de, sub_obj.id);
                    end
                    % obj = retrieve_scalar(de, obj_t);
                case DAEC.enums.class_t.class_scalar
                    obj = retrieve_scalar(de, obj_t);
                case DAEC.enums.class_t.class_vector
                    obj = retrieve_vector(de, obj_t);
                case DAEC.enums.class_t.class_matrix
                    obj = retrieve_matrix(de, obj_t);
                case DAEC.enums.class_t.class_ndtseries
                    obj = retrieve_ndarray(de, obj_t);
                otherwise
                    error('unknown object class')
            end           
        end

        function write(de, name, val, pid)
            if nargin == 2
                % structure passed, store as parent catalog
                val = name;
                name = '';
                pid = 0;
            end
            if nargin == 3
                pid = 0
            end

            if isa(val, 'struct')
                new_pid = pid;
                if length(name) > 0
                    % make catalog object
                    de.ensure_writeable(name);
                    id_ptr = libpointer('int64Ptr', 0);
                    [~, ~, new_pid] = DAEC.check_call('de_new_catalog', de.ptr, pid, char(name), id_ptr);
                end
                for f = fieldnames(val)'
                    write(de, f{1}, val.(f{1}), new_pid);
                end
            elseif(isa(val, 'DESeries'))
                 [~] = store_daecseries(de, name, val, pid);
            elseif(isa(val, 'Series')) %iris tseries
                 [~] = store_irisseries(de, name, val, pid);
            elseif(isa(val, 'tseries')) %iris tseries
                 [~] = store_iristseries(de, name, val, pid);
            elseif isscalar(val)
                [~] = store_scalar(de, name, val, pid);
            elseif isvector(val) && ~ischar(val) && ~isstring(val) && size(val,2) == 1
                [~] = store_matrix(de, name, val, pid);
            elseif isnumeric(val) || islogical(val)
                [~] = store_matrix(de, name, val, pid);
            elseif ischar(val) || isstring(val)
                [~] = store_scalar(de, name, val, pid);
            else
                warning(sprintf('Skipping writing of %s. Unsupported type.\n', name))
            end
        end

        function val = retrieve_scalar(de, obj_t)
            scalarStruct = libstruct('scalar_t');
            scalarStruct.object = obj_t;
            scalarPtr = libpointer('scalar_t', scalarStruct);
            [~, scalar_t] = DAEC.check_call('de_load_scalar', de.ptr, obj_t.id, scalarPtr);
            switch DAEC.enums.type_t.(obj_t.obj_type)
                case DAEC.enums.type_t.type_float
                    val = DAEC.call('get_double_from_voidptr', scalar_t.value);
                case DAEC.enums.type_t.type_signed
                    val = int64(DAEC.call('get_int64_from_voidptr', scalar_t.value));
                case DAEC.enums.type_t.type_unsigned
                    val = uint64(DAEC.call('get_uint64_from_voidptr', scalar_t.value));
                case DAEC.enums.type_t.type_string
                    val = DAEC.call('get_string_from_voidptr', scalar_t.value);
                    if isempty(val)
                        val = '';
                    else
                        val = char(val);  % Convert to MATLAB string
                        % Clean up any null terminators
                        null_idx = find(val == 0, 1);
                        if ~isempty(null_idx)
                            val = val(1:null_idx-1);
                        end
                    end
                case DAEC.enums.type_t.type_complex
                    real_part = DAEC.call('get_complex_real_from_voidptr', scalar_t.value);
                    imag_part = DAEC.call('get_complex_imag_from_voidptr', scalar_t.value);
                    val = complex(real_part, imag_part);
                case DAEC.enums.type_t.type_date
                    val = int64(DAEC.call('get_int64_from_voidptr', scalar_t.value));
                    freq = DAEC.enums.frequency_t.(scalar_t.frequency);
                    if freq ~= DAEC.enums.frequency_t.freq_none
                        val = DEDate(freq, val);
                    end
                otherwise
                    error(sprintf('unsupported scalar type %s', obj_t.obj_type))
            end
        end

        function id = store_scalar(de, name, value, pid)
            if nargin < 4
                pid = 0;
            end

            de.ensure_writeable(name);

            id_ptr = libpointer('int64Ptr', 0);
            [type, freq, val_ptr, nbytes] = DAEC.prepare_scalar(value);

            [~, ~, ~, id] = DAEC.check_call('de_store_scalar', de.ptr, pid, char(name), type, freq, nbytes, val_ptr, id_ptr);

        end

        function val = retrieve_vector(de, obj_t)
            tseries_struct = libstruct('tseries_t');
            axis_struct = libstruct('axis_t');
            tseries_struct.object = obj_t;
            tseries_struct.axis = axis_struct;
            tseries_ptr = libpointer('tseries_t', tseries_struct);

            [~, tseries_t] = DAEC.check_call('de_load_tseries', de.ptr, obj_t.id, tseries_ptr);
            elfreq = DAEC.enums.frequency_t.(tseries_t.elfreq);
            data_shape = [tseries_t.axis.length 1];

            data = DAEC.extract_array_data(tseries_t.value, DAEC.enums.type_t.(tseries_t.eltype), data_shape);
            
            if elfreq ~= DAEC.enums.frequency_t.freq_none
                data = DAEC.to_date_array(data, elfreq, data_shape);
            end

            switch  DAEC.enums.axis_type_t.(tseries_t.axis.ax_type)
                case DAEC.enums.axis_type_t.axis_plain
                    val = data;
                case DAEC.enums.axis_type_t.axis_range
                    if de.read_to_iris
                        attr = get_all_attributes(de, obj_t.id);
                        val = DAEC.make_iris_series(DEAxis(tseries_t.axis), data, attr);
                    else
                        val = DESeries(DEAxis(tseries_t.axis), data);
                    end
                case DAEC.enums.axis_type_t.axis_names
                    % todo: make something
                    val = data;
                otherwise 
                    error(sprintf('unsupported axis type type %s', tseries_t.axis.ax_type))
            end
        end

        function val = retrieve_matrix(de, obj_t)
            mvtseries_struct = libstruct('mvtseries_t');
            axis1_struct = libstruct('axis_t');
            axis2_struct = libstruct('axis_t');
            mvtseries_struct.object = obj_t;
            mvtseries_struct.axis1 = axis1_struct;
            mvtseries_struct.axis2 = axis2_struct;
            mvtseries_ptr = libpointer('mvtseries_t', mvtseries_struct);

            [~, mvtseries_t] = DAEC.check_call('de_load_mvtseries', de.ptr, obj_t.id, mvtseries_ptr);
            elfreq = DAEC.enums.frequency_t.(mvtseries_t.elfreq);

            data_shape = [mvtseries_t.axis1.length mvtseries_t.axis2.length];
            
            data = DAEC.extract_array_data(mvtseries_t.value, DAEC.enums.type_t.(mvtseries_t.eltype), data_shape);
            
            if elfreq ~= DAEC.enums.frequency_t.freq_none
                data = DAEC.to_date_array(data, elfreq, data_shape);
            end

            if DAEC.enums.axis_type_t.(mvtseries_t.axis1.ax_type) == DAEC.enums.axis_type_t.axis_plain && DAEC.enums.axis_type_t.(mvtseries_t.axis2.ax_type) == DAEC.enums.axis_type_t.axis_plain
                % both axes plain
                val = data;
            elseif de.read_to_iris
                attr = get_all_attributes(de, obj_t.id);
                val = DAEC.make_iris_series([DEAxis(mvtseries_t.axis1), DEAxis(mvtseries_t.axis2)], data, attr);
            else
                val = DESeries([DEAxis(mvtseries_t.axis1), DEAxis(mvtseries_t.axis2)], data);
            end
        end

        function val = retrieve_ndarray(de, obj_t)

            % get axis ids
            axis_ids_ptr = libpointer('int64Ptr', repmat(0, 1, DAEC.max_axes));
            [~, axis_ids, ] = DAEC.check_call('de_load_ndtseries_axis_ids', de.ptr, obj_t.id, axis_ids_ptr);

            % load axes
            axis = DEAxis.empty;
            data_shape = [];
            non_plain = false;
            for i = 1:length(axis_ids)
                if axis_ids(i) == -1
                    break;
                end
                axis_struct = libstruct('axis_t');
                axis_struct.ax_type = int32(0);
                axis_struct.frequency = int32(0);
                axis_ptr = libpointer('axis_t', axis_struct);
                [~, axis_t] = DAEC.check_call('de_load_axis', de.ptr, axis_ids(i), axis_ptr);
                axis(i) = DEAxis(axis_t);
                if axis(i).ax_type ~= DAEC.enums.axis_type_t.axis_plain
                    non_plain = true;
                end
                data_shape(1, i) = axis(i).length;
            end
            if length(data_shape) == 1
                data_shape = [data_shape 1];
            end
            
            % get eltype, elfreq
            eltype_ptr = libpointer('type_t', 0);
            elfreq_ptr = libpointer('frequency_t', 0);
            [~, eltype_str, elfreq_str] = DAEC.check_call('de_load_ndtseries_eltype_elfreq', de.ptr, obj_t.id, eltype_ptr, elfreq_ptr);

            eltype = DAEC.enums.type_t.(eltype_str);
            elfreq = DAEC.enums.frequency_t.(elfreq_str);
            
            % get value
            value_ptr = libpointer('voidPtrPtr', 0);
            [~, value] = DAEC.check_call('de_load_ndtseries_value', de.ptr, obj_t.id, value_ptr);
            data = DAEC.extract_array_data(value_ptr, eltype, data_shape);

            if elfreq ~= DAEC.enums.frequency_t.freq_none
                data = DAEC.to_date_array(data, elfreq, data_shape);
            end

            if non_plain
                val = DESeries(axis, data);
            else
                val = data;
            end
        end

        function id = store_matrix(de, name, value, pid)
            if nargin < 4
                pid = 0;
            end

            de.ensure_writeable(name);

            id_ptr = libpointer('int64Ptr', 0);
            [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(value(:));
            
            val_size = size(value);
            if val_size(2) == 1
                % vector
                axis_id = de.create_axis(DAEC.enums.frequency_t.freq_none, val_size(1), 1);
                [~, ~, ~, id] = DAEC.check_call('de_store_tseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_vector, eltype, elfreq, axis_id, nbytes, val_ptr, id_ptr);
            elseif length(val_size) == 2
                % matrix
                axis_id1 = de.create_axis(DAEC.enums.frequency_t.freq_none, val_size(1), 1);
                axis_id2 = de.create_axis(DAEC.enums.frequency_t.freq_none, val_size(2), 1);
                [~, ~, ~, id] = DAEC.check_call('de_store_mvtseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_matrix, eltype, elfreq, axis_id1, axis_id2, nbytes, val_ptr, id_ptr);
            else                
                naxes = length(val_size);
                axis_ids = [];
                for i = 1:naxes
                    axis_ids(i) = de.create_axis(DAEC.enums.frequency_t.freq_none, val_size(i), 1);
                end
                axis_ids_ptr = libpointer('int64Ptr', int64(axis_ids(:)));
                % TODO: store as type_tensor
                [~, ~, ~, ~, id] = DAEC.check_call('de_store_ndtseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_ndtseries, eltype, elfreq, naxes, axis_ids_ptr, nbytes, val_ptr, id_ptr);
            end
            % type = DAEC.enums.class_t.class_vector;
            


        end

        function id = store_tseries(de, name, value, pid)
            if nargin < 4
                pid = 0;
            end

            de.ensure_writeable(name);

            id_ptr = libpointer('int64Ptr', 0);
            [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(value.values(:));
            
            val_size = size(value.values);
            if val_size(2) == 1
                % tseries
                axis_id = de.create_axis(value.start.frequency, val_size(1), value.start.value);
                [~, ~, ~, id] = DAEC.check_call('de_store_tseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_tseries, eltype, elfreq, axis_id, nbytes, val_ptr, id_ptr);
            else
                error('Too many dimensions!')
            end
            % type = DAEC.enums.class_t.class_vector;
        end       

        function id = store_daecseries(de, name, series, pid)
            if nargin < 4
                pid = 0;
            end

            de.ensure_writeable(name);

            id_ptr = libpointer('int64Ptr', 0);
            
            if numel(series.axis) == 1 && series.axis(1).ax_type == DAEC.enums.axis_type_t.axis_range; 
                % tseries
                [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(series.value(:));
                axis_id = de.create_axis(series.axis(1).frequency, series.axis(1).length, series.axis(1).first);
                [~, ~, ~, id] = DAEC.check_call('de_store_tseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_tseries, eltype, elfreq, axis_id, nbytes, val_ptr, id_ptr);
            elseif numel(series.axis) == 2 && series.axis(1).ax_type == DAEC.enums.axis_type_t.axis_range && series.axis(2).ax_type == DAEC.enums.axis_type_t.axis_names
                % mvtseries
                axis_id1 = de.create_axis(series.axis(1).frequency, series.axis(1).length, series.axis(1).first);
                axis_id2 = de.create_names_axis(series.axis(2).names);
                [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(series.value(:));
                [~, ~, ~, id] = DAEC.check_call('de_store_mvtseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_mvtseries, eltype, elfreq, axis_id1, axis_id2, nbytes, val_ptr, id_ptr);
            else
                naxes = numel(series.axis);
                axis_ids = [];
                for i = 1:naxes
                    if series.axis(i).ax_type == DAEC.enums.axis_type_t.axis_names
                        axis_ids(i) = de.create_names_axis(series.axis(i).names);
                    else
                        axis_ids(i) = de.create_axis(series.axis(i).frequency, series.axis(i).length, series.axis(i).first);
                    end
                end
                axis_ids_ptr = libpointer('int64Ptr', int64(axis_ids(:)));
                [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(series.value(:));
                [~, ~, ~, ~, id] = DAEC.check_call('de_store_ndtseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_ndtseries, eltype, elfreq, naxes, axis_ids_ptr, nbytes, val_ptr, id_ptr);
            end
            % type = DAEC.enums.class_t.class_vector;
        end 
        
        function id = store_iristseries(de, name, series, pid)
            if nargin < 4
                pid = 0;
            end

            de.ensure_writeable(name);

            id_ptr = libpointer('int64Ptr', 0);
            [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(series.data(:));
            
            val_size = size(series.data);
            if val_size(2) == 1
                start_date = DAEC.daec_from_iris_date(series);
                % tseries
                axis_id = de.create_axis(start_date.frequency, val_size(1), start_date.value);
                [~, ~, ~, id] = DAEC.check_call('de_store_tseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_tseries, eltype, elfreq, axis_id, nbytes, val_ptr, id_ptr);
                if isprop(series, 'Comment') && ~isempty(char(series.Comment))
                    set_attribute(de, id, 'Comment', char(series.Comment))
                end
            else
                start_date = DAEC.daec_from_iris_date(series);
                axis_id1 = de.create_axis(start_date.frequency, val_size(1), start_date.value);
                if iscell(series.Comment)
                    axis_id2 = de.create_names_axis(series.Comment);
                else
                    error('IRIS tseries has multiple dimensions but Comment field is not a cell array; cannot store names axis.')
                end
                [~, ~, ~, id] = DAEC.check_call('de_store_mvtseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_mvtseries, eltype, elfreq, axis_id1, axis_id2, nbytes, val_ptr, id_ptr);
            end
        end

        function id = store_irisseries(de, name, series, pid)
            if nargin < 4
                pid = 0;
            end

            de.ensure_writeable(name);

            id_ptr = libpointer('int64Ptr', 0);
            [eltype, elfreq, val_ptr, nbytes] = DAEC.prepare_scalar(series.data(:));
            
            val_size = size(series.data);
            if val_size(2) == 1
                start_date = DAEC.daec_from_iris_date(series);
                % tseries
                axis_id = de.create_axis(start_date.frequency, val_size(1), start_date.value);
                [~, ~, ~, id] = DAEC.check_call('de_store_tseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_tseries, eltype, elfreq, axis_id, nbytes, val_ptr, id_ptr);
                set_attribute(de, id, 'iris_type', 'S');
                if isprop(series, 'Comment') && ~isempty(char(series.Comment))
                    set_attribute(de, id, 'Comment', char(series.Comment))
                end
            else
                start_date = DAEC.daec_from_iris_date(series);
                axis_id1 = de.create_axis(start_date.frequency, val_size(1), start_date.value);
                if ~isempty(de.iris_colnames_field)
                    axis_id2 = de.create_names_axis(series.UserData.(de.iris_colnames_field));
                elseif iscell(series.Comment)
                    axis_id2 = de.create_names_axis(series.Comment);
                else
                    error('IRIS series has multiple dimensions but could not find column names; cannot store names axis. Try specifying iris_colnames_field when opening the file.')
                end
                [~, ~, ~, id] = DAEC.check_call('de_store_mvtseries', de.ptr, pid, char(name), DAEC.enums.type_t.type_mvtseries, eltype, elfreq, axis_id1, axis_id2, nbytes, val_ptr, id_ptr);
                set_attribute(de, id, 'iris_type', 'S');
                if isempty(de.iris_colnames_field)
                    set_attribute(de, id, 'iris_colnames_field', 'Comment');
                elseif isprop(series, 'Comment') && ~isempty(char(series.Comment))
                    set_attribute(de, id, 'Comment', char(series.Comment));
                end
                for f = fieldnames(series.UserData)'
                    if strcmp(f{1}, de.iris_colnames_field) == 1
                        set_attribute(de, id, 'iris_colnames_field', char(f{1}));
                        continue
                    elseif ischar(series.UserData.(f{1})) || isstring(series.UserData.(f{1}))
                        set_attribute(de, id, f{1}, char(series.UserData.(f{1})));
                    else
                        warning(sprintf('Skipping storing of UserData field %s for object %s. Unsupported type.\n', f{1}, name))
                    end
                end
            end
        end

    end

    methods % write helpers
        function ensure_writeable(de, name)
            if de.isopen() == false
                error(sprintf('DE file not opened.'))
            end

            if de.readonly == true
                error(sprintf('Cannot write to DE file opened in read only mode.'))
            end

            if ~ischar(name) && ~isstring(name)
                error(sprintf('Object name must be a string or char array.'))
            end
        end

        function axis_id = create_axis(de, freq, len, start_val)
            axis_id_ptr = libpointer('int64Ptr', int64(0));
            if freq == DAEC.enums.frequency_t.freq_none
                [~, axis_id] = DAEC.check_call('de_axis_plain', de.ptr, int64(len), axis_id_ptr);
            else
                [~, axis_id] = DAEC.check_call('de_axis_range', de.ptr, int64(len), freq, int64(start_val), axis_id_ptr);
            end
        end

        function axis_id = create_names_axis(de, names)
            axis_id_ptr = libpointer('int64Ptr', int64(0));
            names_packed = DAEC.pack_column_names(names);
            [~, ~, axis_id] = DAEC.check_call('de_axis_names', de.ptr, int64(numel(names)), names_packed, axis_id_ptr);
        end

        function set_attribute(de, obj_id, name, value)
            [~] = DAEC.check_call('de_set_attribute', de.ptr, obj_id, char(name), char(value));
        end
  
    end

end