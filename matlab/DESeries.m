classdef DESeries < handle
% DESeries - Time series class for DataEcon

    properties (Access = public)
        axis {mustBeA(axis, 'DEAxis')} = DEAxis(0,0,0,0)
        value {mustBeA(value, ["double", "int64", "uint64", "logical", "DEDate"])} = []
    end

    properties (Dependent)
        naxes = numel(obj.axis)
        eltype = DAEC.enums.type_t_by_matlab_class.(class(obj.value))
        elfreq
    end

    methods
        function obj = DESeries(varargin)

            % Default initialization
            obj.axis = varargin{1};
            obj.value = varargin{2};

            % Validate dimensions match
            naxes = numel(obj.axis);
            dims = [];
            for dim = size(obj.value)
                if dim > 1
                    dims = [dims dim];
                end
            end

            if length(dims) ~= naxes
                error('DESeries:DimensionMismatch', ...
                    'Value has %d dimensions but %d axes provided', length(dims), naxes);
            end
            for i = 1:naxes
                if  dims(i) ~= obj.axis(i).length
                    error('DESeries:DimensionMismatch', ...
                            'Axis %d: expected length %d, got %d', ...
                            i, obj.axis(i).length, dims(i));
                end
            end

            % store 1d as column vectors
            if naxes == 1 
                size_ = size(obj.value);
                if size_(1) == 1
                    obj.value = obj.value';
                end
            end

        end

        function val = get.elfreq(obj)
            if obj.eltype ~= 3
                val = DAEC.enums.frequency_t.freq_none;
            elseif isa(obj.value, 'MIT')
                val = obj.value.frequency;
            elseif isa(obj.value, 'datetime')
                val = DAEC.enums.frequency_t.freq_daily;
            else
                error('Unsupported value property for eltype = 3')
            end
        end

        function disp(obj)
            naxes = numel(obj.axis);

            % Case 1: All axes are plain - just display the array
            all_plain = true;
            for i = 1:naxes
                if obj.axis(i).ax_type ~= DAEC.enums.axis_type_t.axis_plain
                    all_plain = false;
                    break;
                end
            end

            if all_plain
                disp(obj.value);
                return;
            end

            % Display axis summary for non-plain cases
            obj.display_axis_summary();

            % Case 2: Single axis (1D)
            if naxes == 1
                if obj.axis(1).ax_type == DAEC.enums.axis_type_t.axis_range
                    % Time series with dates
                    fprintf('\n');
                    for i = 1:obj.axis(1).length
                        date = DEDate(obj.axis(1).frequency, obj.axis(1).first + i - 1);
                        if isa(obj.value(i), 'DEDate')
                            fprintf('  %s    %s\n', format(date), format(obj.value(i)));
                        else
                            fprintf('  %s    %g\n', format(date), obj.value(i));
                        end
                    end
                    fprintf('\n');
                elseif obj.axis(1).ax_type == DAEC.enums.axis_type_t.axis_names
                    % Vector with names
                    T = array2table(obj.value(:), 'VariableNames', {'Value'}, 'RowNames', obj.axis(1).names);
                    fprintf('\n');
                    disp(T);
                else
                    % Plain axis (shouldn't reach here due to earlier check)
                    fprintf('\n');
                    disp(obj.value);
                    fprintf('\n');
                end
                return;
            end

            % Case 3: Two axes (2D)
            if naxes == 2
                fprintf('\n');
                obj.display_2d_slice(obj.value, obj.axis(1), obj.axis(2));
                return;
            end

            % Case 4: Three dimensions - show slices along 3rd dimension
            if naxes == 3

                % Determine how many slices to show (max 5)
                max_slices = 5;
                num_slices = min(obj.axis(3).length, max_slices);

                for slice_idx = 1:num_slices
                    fprintf('\n--- ');

                    % Label the slice based on axis type
                    if obj.axis(3).ax_type == DAEC.enums.axis_type_t.axis_range
                        date = DEDate(obj.axis(3).frequency, obj.axis(3).first + slice_idx - 1);
                        fprintf('Slice %d: %s ---\n', slice_idx, format(date));
                    elseif obj.axis(3).ax_type == DAEC.enums.axis_type_t.axis_names
                        fprintf('Slice %d: %s ---\n', slice_idx, obj.axis(3).names{slice_idx});
                    else
                        fprintf('Slice %d ---\n', slice_idx);
                    end

                    % Extract 2D slice
                    slice_data = obj.value(:, :, slice_idx);

                    % Display slice based on first two axes
                    obj.display_2d_slice(slice_data, obj.axis(1), obj.axis(2));
                end

                % Indicate if there are more slices
                if obj.axis(3).length > max_slices
                    fprintf('\n... (%d more slices not shown)\n', obj.axis(3).length - max_slices);
                end
                fprintf('\n');
                return;
            end

            % Case 5: Four or more dimensions - show summary only
            if naxes >= 4
                fprintf('\n(Data display not implemented for %d dimensions)\n\n', naxes);
                return;
            end

            % Default: for other cases not yet handled, just show the value
            fprintf('\nDESeries (display not implemented for this configuration)\n');
            disp(obj.value);
        end

    end

    methods (Access = private)
        function display_axis_summary(obj)
            % Display summary information about each axis
            naxes = numel(obj.axis);

            fprintf('\nDESeries with %d ', naxes);
            % fprintf('\nDESeries with 3 axes (%d × %d × %d)\n', ...
            %         obj.axis(1).length, obj.axis(2).length, obj.axis(3).length);
            if naxes == 1
                fprintf('axis');
            else    
                fprintf('axes');
            end
            fprintf(' (');
            for i = 1:naxes
                fprintf('%d', obj.axis(i).length);
                if i < naxes
                    fprintf(' × ');
                end
            end
            fprintf(')\n');

            % Show information for each axis
            for i = 1:naxes
                fprintf('  Axis %d: ', i);
                if obj.axis(i).ax_type == DAEC.enums.axis_type_t.axis_plain
                    fprintf('plain [1:%d]\n', obj.axis(i).length);
                elseif obj.axis(i).ax_type == DAEC.enums.axis_type_t.axis_range
                    first_date = DEDate(obj.axis(i).frequency, obj.axis(i).first);
                    last_date = DEDate(obj.axis(i).frequency, obj.axis(i).first + obj.axis(i).length - 1);
                    fprintf('%s to %s\n', format(first_date), format(last_date));
                elseif obj.axis(i).ax_type == DAEC.enums.axis_type_t.axis_names
                    if obj.axis(i).length <= 5
                        fprintf('%s\n', strjoin(obj.axis(i).names, ', '));
                    else
                        fprintf('%s, ... (%d total)\n', ...
                            strjoin(obj.axis(i).names(1:3), ', '), obj.axis(i).length);
                    end
                end
            end
        end

        function display_2d_slice(~, slice_data, axis1, axis2)
            % Helper method to display a 2D slice based on axis types

            % Convert DEDate arrays to formatted strings for table display
            if isa(slice_data, 'DEDate')
                display_data = cell(size(slice_data));
                for i = 1:numel(slice_data)
                    display_data{i} = format(slice_data(i));
                end
            else
                display_data = slice_data;
            end

            % Case: axis_range × axis_names (multivariate time series)
            if axis1.ax_type == DAEC.enums.axis_type_t.axis_range && ...
               axis2.ax_type == DAEC.enums.axis_type_t.axis_names

                dates = cell(axis1.length, 1);
                for i = 1:axis1.length
                    date = DEDate(axis1.frequency, axis1.first + i - 1);
                    dates{i} = format(date);
                end
                T = array2table(display_data, 'VariableNames', axis2.names, 'RowNames', dates);
                disp(T);

            % Case: axis_names × axis_range (transposed time series)
            elseif axis1.ax_type == DAEC.enums.axis_type_t.axis_names && ...
                   axis2.ax_type == DAEC.enums.axis_type_t.axis_range

                dates = cell(axis2.length, 1);
                for i = 1:axis2.length
                    date = DEDate(axis2.frequency, axis2.first + i - 1);
                    dates{i} = format(date);
                end
                T = array2table(display_data, 'VariableNames', dates, 'RowNames', axis1.names);
                disp(T);

            % Case: axis_names × axis_names
            elseif axis1.ax_type == DAEC.enums.axis_type_t.axis_names && ...
                   axis2.ax_type == DAEC.enums.axis_type_t.axis_names

                T = array2table(display_data, 'VariableNames', axis2.names, 'RowNames', axis1.names);
                disp(T);

            % Default: just display the numeric array
            else
                disp(slice_data);
            end
        end
    end
end