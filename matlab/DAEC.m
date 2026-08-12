classdef DAEC < handle

    methods (Static, Access=private)
        function inst = instance()
            persistent instance_;    % the singleton instance
            if isempty(instance_) || ~isvalid(instance_)
                instance_ = DAEC();
            end
            inst = instance_;
        end
    end
    
    properties
        libname
        debug_
    end

    properties (Constant)
        enums = daecenums(); % daecenums()
    end

    methods (Access=private)
        % private constructor, so it doesn't get called willy-nilly
        function inst = DAEC()
            inst.libname = '';
            switch computer
                case {'PCWIN', 'PCWIN64'}
                    inst.libname = 'libdaec';
                case {'GLNX86', 'GLNXA64'}
                    inst.libname = 'libdaec';
            end
            inst.debug_ = false;
        end
    end

    methods (Static)

        function daec = load(daecroot)
            arguments
                daecroot {mustBeFolder} = '.';
            end
            % nothing to do if library is already loaded
            daec = DAEC.instance;
            if libisloaded(daec.libname)
                return
            end
            % find the library and load it
            hpath = fullfile(daecroot,'include', 'daec.h');

            switch computer()
                case {'PCWIN', 'PCWIN64'}
                    libpath = fullfile(daecroot, 'bin', 'libdaec.dll');
                case {'GLNX86', 'GLNXA64'}
                    libpath = fullfile(daecroot, 'lib', 'libdaec.so');
                otherwise
                    error('DataEcon not supported on your platform.');
            end
            % capture output args to suppress warnings
            warning('off','MATLAB:loadlibrary:TypeNotFoundForStructure');
            [~,~] = loadlibrary(libpath, hpath, 'alias', 'libdaec');
            warning('on','MATLAB:loadlibrary:TypeNotFoundForStructure');
            % [~,~] = loadlibrary(libpath, hpath, mfilename='daecinfo');
            if not(strcmp(daec.enums.version, DAEC.version))
                warning('Incompatible DAEC version.')
            end
        end

        function unload()
            libname = DAEC.instance.libname;
            if libisloaded(libname)
                unloadlibrary(libname)
            end
        end

        function tf = isloaded()
            tf = libisloaded(DAEC.instance.libname);
        end

        function oldval = debug(tf)
            inst = DAEC.instance;
            oldval = inst.debug_;
            if exist('tf', 'var') == false
                inst.debug_ = true;
            else
                inst.debug_ = tf;
            end
        end
        
        function db = readdb(path, NameValueArgs)
            arguments 
                path {mustBeTextScalar} = ''
                NameValueArgs.memory (1,1) {mustBeNumericOrLogical} = false
                NameValueArgs.read_to_iris (1,1) {mustBeNumericOrLogical} = false
                NameValueArgs.read_to_tse (1,1) {mustBeNumericOrLogical} = false
                NameValueArgs.iris_colnames_field {mustBeTextScalar} = ''
            end

            de = DEFile(path, 'memory', NameValueArgs.memory, 'readonly', true, 'read_to_iris', NameValueArgs.read_to_iris, 'read_to_tse', NameValueArgs.read_to_tse, 'iris_colnames_field', NameValueArgs.iris_colnames_field);
            db = de.read();
            de.close();
        end
        
        function writedb(path, db, NameValueArgs)
            arguments 
                path {mustBeTextScalar} = ''
                db {mustBeA(db, ["struct"])} = struct()
                NameValueArgs.iris_colnames_field {mustBeTextScalar} = ''
            end
            de = DEFile(path,  'readonly', false, 'iris_colnames_field', NameValueArgs.iris_colnames_field);
            de.truncate();
            de.write(db);
            de.close();
            
        end
    end

    methods (Static)
        function ver = version()
            ver = calllib(DAEC.instance.libname, 'de_version');
        end

        function de = open(varargin)
            de = DEFile(varargin{:});
        end

        function max_axes = max_axes()
            max_axes = calllib(DAEC.instance.libname, 'de_max_axes');
        end

        function old = old_iris()
            vstring = irisversion;
            year = str2double(vstring(1:4));
            old = year <= 2015
        end
    end

    methods (Static)
        % https://www.mathworks.com/help/matlab/call-c-library-functions.html
        % https://www.mathworks.com/help/matlab/matlab_external/passing-arguments-to-shared-library-functions.html

        function varargout = check_call(func, varargin)
            varargout = cell(1, nargout);
            [status, varargout{:}] = DAEC.call(func, varargin{:});
            DAEC.check(status);
        end

        function varargout = call(func, varargin)
            inst = DAEC.instance;
            varargout = cell(1, nargout);
            [varargout{:}] = calllib(inst.libname, func, varargin{:});           
        end

        function varargout = call_shim(func, varargin)
            inst = DAEC.instance;
            varargout = cell(1, nargout);
            [varargout{:}] = calllib(inst.shimname, func, varargin{:});           
        end

        function check(status)
            if (exist('status', 'var') > 0) && (status == 0)
                return
            end
            msg = repmat(' ', 1, 512);
            if DAEC.instance.debug_
                [status, msg] = calllib(DAEC.instance.libname, 'de_error_source', msg, numel(msg)-1);
            else
                [status, msg] = calllib(DAEC.instance.libname, 'de_error', msg, numel(msg)-1);
            end
            if status ~= 0
                error(msg);
            end
        end
    end

    methods (Static) % write helpers
        function [type, freq, val_ptr, nbytes] = prepare_scalar(value)
            nvals = numel(value);
            freq = DAEC.enums.frequency_t.freq_none;
            nbytes = nvals * 8;
            type = 0;
            if isfield(DAEC.enums.type_t_by_matlab_class, class(value))
                type = DAEC.enums.type_t_by_matlab_class.(class(value));
            end
            switch type
                case 1 % integer
                    val_ptr = libpointer('int64Ptr', int64(value));
                case 2 % unsigned int
                    val_ptr = libpointer('int64Ptr', int64(value));
                case 3 % date
                    if isa(value, 'datetime')
                        freq = DAEC.enums.frequency_t.freq_daily;
                        [year, month, day] = ymd(value);
                        date_ptr = libpointer('int64Ptr', int64(0));
                        val_ptr = DAEC.check_call('de_pack_calendar_date', freq, int32(year), uint32(month), uint32(day), date_ptr);
                    else
                        % DEDate array - extract frequency and values
                        freq = value(1).frequency;
                        if nvals == 1
                            vals = int64(value.value);
                        else
                            % Preallocate and extract values efficiently
                            vals = zeros(size(value), 'int64');
                            for i = 1:numel(value)
                                vals(i) = value(i).value;
                            end
                        end
                        val_ptr = libpointer('int64Ptr', vals);
                    end
                case 4 % double
                    if isreal(value)
                        val_ptr = libpointer('doublePtr', double(value));
                    else
                        type = DAEC.enums.type_t.type_complex;
                        convertedValue = complex(double(real(value)), double(imag(value)));
                        val_ptr = libpointer('doublePtr', [real(convertedValue), imag(convertedValue)]);
                        nbytes = nvals* 16;
                    end
                case 6 % string
                    char_val = char(value);
                    % Ensure null termination
                    if isempty(char_val) || char_val(end) ~= char(0)
                        char_val = [char_val, char(0)];
                    end
                    val_ptr = libpointer('cstring', char_val);
                    nbytes = length(char_val);
                    % todo: vector of strings?
                otherwise
                    if isfloat(value) && isscalar(value)
                        type = 4;
                        val_ptr = libpointer('doublePtr', double(value));
                    else
                        error('Unsupported value type %s', value)
                    end
            end
        end

        function packed_names = pack_column_names(names)
            if isempty(names)
                packed_names = '';
                return;
            end
            
            % Convert to cell array of chars for consistency
            packed_names = strjoin(names, char(10));  % char(10) = '\n'
        end

        function daec_date = daec_from_iris_date(ts) 
            old_iris = false;
            if isprop(ts, 'Start')
                % modern iris
                freq = DAEC.enums.frequency_convert.from_iris(double(ts.Frequency));
                val = double(ts.Start);
            else
                old_iris = true;
                % extract frequency from digits
                val = floor(ts.start);
                freq_stub = round((ts.start - val) * 100);
                if freq_stub == 0
                    freq_stub = 365;
                end
                freq = DAEC.enums.frequency_convert.from_iris(freq_stub);
            end
            
            switch freq
                case {  DAEC.enums.frequency_t.freq_yearly_dec, 
                        DAEC.enums.frequency_t.freq_halfyearly_jun,
                        DAEC.enums.frequency_t.freq_quarterly_mar,
                        DAEC.enums.frequency_t.freq_monthly
                    }
                    daec_date = DEDate(freq, val);
                case  DAEC.enums.frequency_t.freq_weekly_thu
                    if old_iris
                        daec_date = DEDate(freq, datetime(val*7, 'ConvertFrom', 'datenum'));
                    else
                        daec_date = DEDate(freq, ts.Start.datetime);
                    end
                case DAEC.enums.frequency_t.freq_daily
                    if old_iris
                        daec_date = DEDate(freq, datetime(val, 'ConvertFrom', 'datenum'));
                    else
                        daec_date = DEDate(freq, ts.Start.datetime);
                    end
                otherwise
                    daec_date = DEDate(DAEC.enums.frequency_t.freq_unit, val);
            end
        end
    end

    methods (Static) % read helpers
        function data = extract_array_data(val_ptr, eltype, data_shape)
            numel = prod(data_shape);
            switch eltype
                case DAEC.enums.type_t.type_float
                    data = zeros(data_shape, 'double');
                    data_ptr = libpointer('doublePtr', data);
                    [~, data] = DAEC.call('get_double_array_from_voidptr', val_ptr, numel, data_ptr);
                    data = double(data);
                    if length(data_shape) > 2
                        data = reshape(data, data_shape);
                    end
                case DAEC.enums.type_t.type_signed
                    data = zeros(data_shape, 'int64');
                    data_ptr = libpointer('int64Ptr', data);
                    [~, data] = DAEC.call('get_int64_array_from_voidptr', val_ptr, numel, data_ptr);
                    data = int64(data);
                case DAEC.enums.type_t.type_date
                    data = zeros(data_shape, 'int64');
                    data_ptr = libpointer('int64Ptr', data);
                    [~, data] = DAEC.call('get_int64_array_from_voidptr', val_ptr, numel, data_ptr);
                    data = int64(data);
                case DAEC.enums.type_t.type_unsigned
                    data = zeros(data_shape, 'uint64');
                    data_ptr = libpointer('uint64Ptr', data);
                    [~, data] = DAEC.call('get_uint64_array_from_voidptr', val_ptr, numel, data_ptr);
                    data = uint64(data);
                case DAEC.enums.type_t.type_string
                    error("Reading a vector/matrix of strings is not supported...")
                case DAEC.enums.type_t.type_complex
                    adjusted_data_shape = data_shape;
                    adjusted_data_shape(end) = data_shape(end)*2;
                    parted_data = zeros(adjusted_data_shape, 'double');
                    data_ptr = libpointer('doublePtr', parted_data);
                    [~, parted_data] = DAEC.call('get_double_array_from_voidptr', val_ptr, numel*2, data_ptr);
                    parted_data = double(parted_data);
                    if length(data_shape) > 2
                        parted_data = reshape(parted_data, adjusted_data_shape);
                    end
                    real_idx = repmat({':'}, 1, length(data_shape));
                    imag_idx = repmat({':'}, 1, length(data_shape));
                    real_idx{end} = 1:data_shape(end); 
                    imag_idx{end} = (data_shape(end)+1):(data_shape(end) * 2); 
                    data = complex(parted_data(real_idx{:}), parted_data(imag_idx{:}));
                otherwise
                    error(sprintf('unsupported array type %s', eltype))
            end
        end

        function colnames = unpack_column_names(names, ncols)
            
            colnames = {};
            
            if isempty(names) || ncols == 0
                return;
            end
            
            % Split on newline characters (Julia convention)
            if contains(names, char(10))
                colnames = strsplit(names, char(10), 'CollapseDelimiters', false);
            else
                colnames = {names};
            end                
        end

        function data = to_date_array(data, elfreq, data_shape)
            % Preallocate DEDate array with same shape as data
            date_array(numel(data)) = DEDate();
            date_array = reshape(date_array, data_shape);

            % Populate with DEDate objects using vectorized approach
            for i = 1:numel(data)
                date_array(i) = DEDate(elfreq, data(i));
            end

            data = date_array;
        end

        function iris_series = make_iris_series(axes, data, attr)
            make_tseries = true;
            if isfield(attr, 'iris_type') && attr.iris_type == 'S'
                make_tseries = false;
            end
            if numel(axes) == 1
                iris_freq = DAEC.enums.frequency_convert.to_iris(axes.frequency);
                start_date = DAEC.iris_date(iris_freq, axes);
                end_date = start_date + (axes.length - 1);
                if make_tseries
                    iris_series = tseries(start_date:end_date, Inf);
                    iris_series.data = data;
                    if isfield(attr, 'Comment')
                        if isa(attr.Comment, 'char')
                            iris_series.Comment = {attr.Comment};
                        else
                            iris_series.Comment = attr.Comment;
                        end
                    end
                else
                    iris_series = Series(start_date:end_date, data);
                    for f = fieldnames(attr)'
                        if strcmp(f{1}, 'Comment') == 1
                            if isa(attr.Comment, 'char')
                                iris_series.Comment = {attr.Comment};
                            else
                                iris_series.Comment = attr.Comment;
                            end
                        else
                            iris_series.UserData.(f{1}) = attr.(f{1});
                        end
                    end
                end
                
            else
                iris_freq = DAEC.enums.frequency_convert.to_iris(axes(1).frequency);
                start_date = DAEC.iris_date(iris_freq, axes(1));
                end_date = start_date + (axes(1).length - 1);
                if make_tseries
                    iris_series = tseries(start_date:end_date, Inf);
                    iris_series.data = data;
                    iris_series.Comment = axes(2).names;
                else
                    iris_series = Series(start_date:end_date, data);
                    for f = fieldnames(attr)'
                        if strcmp(f{1}, 'Comment') == 1
                            if isa(attr.Comment, 'char')
                                iris_series.Comment = {attr.Comment};
                            else
                                iris_series.Comment = attr.Comment;
                            end
                        elseif strcmp(f{1}, 'iris_colnames_field')
                            iris_series.UserData.(attr.(f{1})) = axes(2).names;
                        else
                            iris_series.UserData.(f{1}) = attr.(f{1});
                        end
                    end 
                end
            end
        end

        function iris_date_obj = iris_date(iris_freq, axis)
            daec_start = axis.first;
            switch iris_freq
                case 1 % yearly
                    iris_date_obj = yy(daec_start);
                case 2 % halfyearly
                    mod = rem(daec_start, 2);
                    iris_date_obj = hh((daec_start-mod)/2, mod+1);
                case 4 % quarterly
                    mod = rem(daec_start, 4);
                    iris_date_obj = qq((daec_start-mod)/4, mod+1);
                case 12 % monthly
                    mod = rem(daec_start, 12);
                    iris_date_obj = mm((daec_start-mod)/12, mod+1);
                case 52 % weekly
                    year_ptr = libpointer('int32Ptr', 0);
                    month_ptr = libpointer('uint32Ptr', 0);
                    day_ptr = libpointer('uint32Ptr', 0);
                    [year, month, day] = DAEC.check_call('de_unpack_calendar_date', axis.frequency, axis.first, year_ptr, month_ptr, day_ptr);
                    iris_date_obj = ww(double(year), double(month), double(day));
                case 365 % daily
                    year_ptr = libpointer('int32Ptr', 0);
                    month_ptr = libpointer('uint32Ptr', 0);
                    day_ptr = libpointer('uint32Ptr', 0);
                    [year, month, day] = DAEC.check_call('de_unpack_calendar_date', axis.frequency, axis.first, year_ptr, month_ptr, day_ptr);
                    iris_date_obj = dd(double(year), double(month), double(day));
                otherwise
                    error(sprintf('No IRIS conversion available for frequency %s', axis.frequency))
            end
        end

    end



end