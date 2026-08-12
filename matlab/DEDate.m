classdef DEDate < handle
% DEDate - Moment-In-Date class for DataEcon date representation

    properties (Access = public)
        frequency   % DataEcon frequency type (integer from Constants)
        value       % Integer date value in DataEcon format
    end

    methods
        function obj = DEDate(varargin)

            % Default initialization
            obj.frequency = DAEC.enums.frequency_t.freq_none;
            obj.value = 0;
            
            if nargin == 0
                return; % Empty DEDate
            end

            % TODO: error messages?

            if ischar(varargin{1})
                switch varargin{1}
                    case 'daily'
                        obj.frequency = DAEC.enums.frequency_t.freq_daily;
                    case 'bdaily'
                        obj.frequency = DAEC.enums.frequency_t.freq_bdaily;
                    case 'weekly'
                        obj.frequency = DAEC.enums.frequency_t.freq_weekly_sun;
                    case 'monthly'
                        obj.frequency = DAEC.enums.frequency_t.freq_monthly;
                    case 'quarterly'
                        obj.frequency = DAEC.enums.frequency_t.freq_quarterly_mar;
                    case 'halfyearly'
                        obj.frequency = DAEC.enums.frequency_t.freq_halfyearly_jun;
                    case 'yearly'
                        obj.frequency = DAEC.enums.frequency_t.freq_yearly_dec;
                    otherwise
                        obj.frequency = DAEC.enums.frequency_t.freq_none;
                end
                if nargin == 2
                    val = varargin{2};
                    if isa(varargin{2}, 'datetime')
                        val_ptr = libpointer('int64Ptr', 0);    
                        date_parts = datevec(varargin{2});
                        val = DAEC.check_call('de_pack_calendar_date', obj.frequency, int32(date_parts(1)), uint32(date_parts(2)), uint32(date_parts(3)), val_ptr);
                    end
                end
                if nargin == 3
                    % frequency and then year, periods
                    val_ptr = libpointer('int64Ptr', 0);
                    val = DAEC.check_call('de_pack_year_period_date', obj.frequency, int32(varargin{2}), uint32(varargin{3}), val_ptr);
                end
                if nargin == 4
                    val_ptr = libpointer('int64Ptr', 0);    
                    val = DAEC.check_call('de_pack_calendar_date', obj.frequency, int32(varargin{2}),uint32(varargin{3}), uint32(varargin{4}), val_ptr);
                end
                obj.value=val;
            elseif nargin == 2 && isscalar(varargin{1}) && isa(varargin{2}, 'datetime')
                % numeric frequency and datetime object (i.e. for IRIS conversion)
                obj.frequency = varargin{1};
                val_ptr = libpointer('int64Ptr', 0);    
                date_parts = datevec(varargin{2});
                val = DAEC.check_call('de_pack_calendar_date', obj.frequency, int32(date_parts(1)), uint32(date_parts(2)), uint32(date_parts(3)), val_ptr);
                obj.value=val;
            else
                obj.frequency = varargin{1};
                obj.value = varargin{2};
            end
        end

        function dateStr = format(obj)
            % Handle array of DEDate objects
            if numel(obj) > 1
                dateStr = cell(size(obj));
                for i = 1:numel(obj)
                    dateStr{i} = format(obj(i));
                end
                return;
            end

            % Handle scalar DEDate
            if obj.frequency == 0
                dateStr = sprintf('%d', obj.value);
            elseif obj.frequency <= 11
                dateStr = sprintf('%dU', obj.value);
            elseif obj.frequency < 32 % daily, bdaily, weekly
                year_ptr = libpointer('int32Ptr', 0);
                month_ptr = libpointer('uint32Ptr', 0);
                day_ptr = libpointer('uint32Ptr', 0);
                [year, month, day] = DAEC.check_call('de_unpack_calendar_date', obj.frequency, obj.value, year_ptr, month_ptr, day_ptr);
                if obj.frequency == 12 % daily
                    dateStr = sprintf('%04d-%02d-%02d', year, month, day);
                elseif obj.frequency == 13 % bdaily
                    dateStr = sprintf('bdaily %04d-%02d-%02d', year, month, day);
                else % weekly
                    dateStr = sprintf('weekly %04d-%02d-%02d', year, month, day);
                end
            elseif obj.frequency < 256 % monthly, quarterly, halfearly
                year_ptr = libpointer('int32Ptr', 0);
                period_ptr = libpointer('uint32Ptr', 0);
                [year, period] = DAEC.check_call('de_unpack_year_period_date', obj.frequency, obj.value, year_ptr, period_ptr);
                if obj.frequency == 32 % monthly
                    dateStr = sprintf('%04dM%02d', year, period);
                elseif obj.frequency < 128 % quarterly
                    dateStr = sprintf('%04dQ%01d', year, period);
                else % halfyearly
                    dateStr = sprintf('%04dH%01d', year, period);
                end
            else
                dateStr = sprintf('%04dY', obj.value);
            end
        end

        function disp(obj)
            % Handle array of DEDate objects
            if numel(obj) > 1
                % Display as array with dimensions
                dims = size(obj);
                fprintf('%d', dims(1));
                for i = 2:length(dims)
                    fprintf('×%d', dims(i));
                end
                fprintf(' DEDate array:\n\n');

                % For 1D arrays, display as list
                if isvector(obj)
                    for i = 1:numel(obj)
                        fprintf('  %s\n', format(obj(i)));
                    end
                % For 2D arrays, display as grid
                elseif ismatrix(obj)
                    for i = 1:size(obj, 1)
                        fprintf('  ');
                        for j = 1:size(obj, 2)
                            fprintf('%-12s  ', format(obj(i, j)));
                        end
                        fprintf('\n');
                    end
                else
                    % For higher dimensions, just show size
                    fprintf('  (Use indexing to view individual dates)\n');
                end
                fprintf('\n');
            else
                % Scalar case
                fprintf('%s\n', format(obj));
            end
        end
    end

    

end