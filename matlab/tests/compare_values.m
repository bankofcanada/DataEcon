function same = compare_values(orig, read, name, verbose)
    if nargin < 4
        verbose = false;
    end
    same = false;
    if isa(orig, 'char') || isa(orig, 'string')
        if strcmp(orig, read) == 1
            same = true;
        else verbose == true
            fprintf('Value difference for %s\n expected: %s\n got:      %s\n', name, orig, read);
        end
    elseif isa(orig, 'tseries')
        if orig.data == read.data
            if double(orig.start) == double(orig.start)
                same = true;
                if isprop(orig, 'Comment') && ~isempty(orig.Comment)
                    if iscell(orig.Comment)
                        for cmt = 1:length(orig.Comment)
                            if strcmp(orig.Comment{cmt}, read.Comment{cmt}) ~= 1
                                same = false;
                                fprintf('Comments difference for %s\n expected: %s\n got:      %s\n', name, orig.Comment{cmt}, read.Comment{cmt})
                            end
                        end
                    end
                end
            else
                fprintf('Start difference for %s\n expected: %d\n got:      %d\n', name, orig.Start, read.Start);
            end
        else
            fprintf('Value difference for %s\n expected: %d\n got:      %d\n', name, orig.data, read.data);
        end
    elseif isa(orig, 'Series')
        if orig.data == read.data
            if double(orig.start) == double(orig.start)
                same = true;
                if isprop(orig, 'Comment') && ~isempty(orig.Comment)
                    if iscell(orig.Comment)
                        for cmt = 1:length(orig.Comment)
                            if strcmp(orig.Comment{cmt}, read.Comment{cmt}) ~= 1
                                same = false;
                                fprintf('Comments difference for %s\n expected: %s\n got:      %s\n', name, orig.Comment{cmt}, read.Comment{cmt})
                            end
                        end
                    end
                end
                if same && isprop(orig, 'userdata') 
                    user_data_same = true;
                    for f = fieldnames(orig.userdata)
                        if iscell(orig.userdata.(f{1}))
                            for ii = 1:numel(orig.userdata.(f{1}))
                                if strcmp(orig.userdata.(f{1}){i}, read.userdata.(f{1}){i}) ~= 1
                                    user_data_same = false;
                                end
                            end
                        else
                            if strcmp(orig.userdata.(f{1}), read.userdata.(f{1})) == 0
                                user_data_same = false;
                            end
                        end
                    end
                    if ~user_data_same
                        fprintf('UserData difference for %s\n', name)
                        same = false;
                    end
                end
            else
                fprintf('Start difference for %s\n expected: %d\n got:      %d\n', name, orig.Start, read.Start);
            end
        else
            fprintf('Value difference for %s\n expected: %d\n got:      %d\n', name, orig.data, read.data);
        end
    elseif isa(orig, 'DESeries')
        % Compare values - handle DEDate arrays specially
        values_same = false;
        if isa(orig.value, 'DEDate')
            % Compare DEDate arrays element by element
            if isa(read.value, 'DEDate') && isequal(size(orig.value), size(read.value))
                values_same = true;
                for i = 1:numel(orig.value)
                    if orig.value(i).frequency ~= read.value(i).frequency || ...
                       orig.value(i).value ~= read.value(i).value
                        values_same = false;
                        break;
                    end
                end
            end
        else
            % Non-DEDate comparison
            values_same = isequal(orig.value, read.value);
        end

        if values_same
            if strcmp(class(orig.value),class(read.value)) == 1

                if numel(orig.axis) == numel(read.axis)
                    axes_same = true;
                    for i = 1:numel(orig.axis)
                        ax_orig = orig.axis(i);
                        ax_read = read.axis(i);
                        if ax_orig.ax_type ~= ax_read.ax_type
                            axes_same = false;
                        elseif ax_orig.frequency ~= ax_read.frequency
                            axes_same = false;
                        elseif ax_orig.length ~= ax_read.length
                            axes_same = false;
                        elseif ax_orig.first ~= ax_read.first
                            axes_same = false;
                        end
                        if axes_same && ax_orig.ax_type == 2
                            for ii = 1:numel(ax_orig.names)
                                if strcmp(ax_orig.names{i}, ax_read.names{i}) ~= 1
                                    axes_same = false;
                                end
                            end
                        end
                    end
                    if axes_same == true
                        same = true;
                    else
                         fprintf('Different axes for %s\n expected: %s\n got:      %s\n', name, orig.axis, read.axis);
                    end
                else
                    fprintf('Different number of axes for %s\n expected: %s\n got:      %s\n', name, numel(orig.axis), numel(read.axis));
                end

            else
                fprintf('Class difference for %s\n expected: %s\n got:      %s\n', name, class(orig.value), class(read.value));
            end
        elseif verbose == true
            fprintf('Value difference for %s\n', name);
            if isa(orig.value, 'DEDate')
                fprintf('  (DEDate arrays differ in frequency or value)\n');
            end
        end
    elseif isa(orig, 'TSeries')
        if orig.values == read.values
            if orig.start.frequency == read.start.frequency
                if orig.start.value == read.start.value
                    same = true;
                else
                    fprintf('Start value difference for %s\n expected: %s\n got:      %s\n', name, orig.start.value, read.start.value);
                end
            else
                fprintf('Start frequency difference for %s\n expected: %s\n got:      %s\n', name, orig.start.frequency, read.start.frequency);
            end
        else verbose == true
            fprintf('Value difference for %s\n expected: %s\n got:      %s\n', name, orig.values, read.values);
        end
    elseif isa(orig, 'MVTSeries')
        if orig.values == read.values
            if orig.start.frequency == read.start.frequency
                if orig.start.value == read.start.value
                    same = true;
                    if numel(orig.names) == numel(read.names)
                        all_colnames_same = true;
                        for i = 1:length(numel(orig.names))
                            if strcmp(orig.names{i}, read.names{i}) ~= 1
                                all_colnames_same = false;
                            end
                        end
                        if all_colnames_same == true
                            same = true;
                        else
                            fprintf('Column names don''t match for %s\n expected: %s\n got:      %s\n', name, orig.names, read.names);
                        end
                        
                    else
                        fprintf('Different number of column names %s\n expected: %s\n got:      %s\n', name, orig.names, read.names);
                    end
                else
                    fprintf('Start value difference for %s\n expected: %s\n got:      %s\n', name, orig.start.value, read.start.value);
                end
            else
                fprintf('Start frequency difference for %s\n expected: %s\n got:      %s\n', name, orig.start.frequency, read.start.frequency);
            end
        else verbose == true
            fprintf('Value difference for %s\n expected: %s\n got:      %s\n', name, orig.values, read.values);
        end
    elseif isa(orig, 'DEDate')
        if isa(read, 'DEDate')
            if orig.frequency == read.frequency
                if orig.value == read.value
                    same = true;
                else
                    fprintf('Value difference for %s\n expected: %s\n got:      %s\n', name, orig.value, read.value);
                end
            else
                fprintf('Frequency difference for %s\n expected: %s\n got:      %s\n', name, orig.frequency, read.frequency);
            end
        else
            fprintf('Class difference for %s\n expected: %s\n got:      %s\n', name, class(orig), class(read));
        end
    elseif orig == read
        if strcmp(class(orig), class(read)) == 1
            same = true;
        elseif strcmp(class(orig), 'logical') == 1 && strcmp(class(read), 'uint64') == 1
             same = true;
        elseif isa(orig, 'DEDate') && orig.frequency == read.frequency && orig.value == read.value
            same = true
        else
            fprintf('Class difference for %s\n expected: %s\n got:      %s\n', name, class(orig), class(read));
        end
    else
        fprintf('Value difference for %s\n expected: %s\n got:      %s\n', name, orig, read);
    end

end