

irisRoot = ''; 
addpath(irisRoot);
iris.startup();

DAEC.load()
test_dir = '';
testFile = fullfile(test_dir, ['test_daec_iris' num2str(randi(1000)) '.daec'])

test_struct = struct();
test_struct.iris_tseries = struct();

gdp_data = 100 + cumsum(0.5 + 0.2*randn(24,1));
test_struct.iris_tseries.quarterly = tseries(qq(2000,1):qq(2005,4), 100 + cumsum(0.5 + 0.2*randn(24,1)));
test_struct.iris_tseries.quarterly.Comment = 'Real GDP (index, q/q cumulative)';

test_struct.iris_tseries.weekly = tseries(ww(2021,1):ww(2021,10), rand(10,1));
test_struct.iris_tseries.daily = tseries(dd(2023,1,1):dd(2023,1,15), 100 + cumsum(0.1 + 0.05*randn(15,1)));
test_struct.iris_tseries.daily.Comment = 'Real GDP (index, daily)';

test_struct.iris_tseries.daily_mvts = tseries(dd(2023,1,1):dd(2023,1,10), [0.1*randn(10,1), 0.05*randn(10,1)]);
test_struct.iris_tseries.daily_mvts.Comment = {'Rate_Change', 'Vol_Index'}; % tseries stores column names in Comment

test_struct.iris_tseries.quarterly_mvts = tseries(qq(2000,1):qq(2005,4), [0.2*randn(24,1), 2 + 0.1*randn(24,1)]);
test_struct.iris_tseries.quarterly_mvts.Comment = {'OutputGap', 'Inflation'}; % tseries stores column names in Comment

% Series object
test_struct.iris_series.quarterly = Series(qq(2000,1):qq(2005,4), 100 + cumsum(0.5 + 0.2*randn(24,1)));
test_struct.iris_series.quarterly.Comment = 'Real GDP (index, q/q cumulative)';
test_struct.iris_series.weekly = Series(ww(2021,1):ww(2021,10), rand(10,1));
test_struct.iris_series.daily = Series(dd(2023,1,1):dd(2023,1,15), 100 + cumsum(0.1 + 0.05*randn(15,1)));

test_struct.iris_series.daily_mvts = Series(dd(2023,1,1):dd(2023,1,10), [0.1*randn(10,1), 0.05*randn(10,1)]);
test_struct.iris_series.daily_mvts.Comment = 'Daily macro indicators: [Rate_Change, Vol_Index]';
test_struct.iris_series.daily_mvts.UserData = struct('Columns', {{'Rate_Change', 'Vol_Index'}});

test_struct.iris_series.quarterly_mvts = Series(qq(2000,1):qq(2002,2), [0.1*randn(10,1), 0.05*randn(10,1)]);
test_struct.iris_series.quarterly_mvts.Comment = 'Daily macro indicators: [Rate_Change, Vol_Index]';
test_struct.iris_series.quarterly_mvts.UserData = struct('Columns', {{'Rate_Change', 'Vol_Index'}});
test_struct.iris_series.quarterly_mvts.UserData.second_comment = 'Hello world!';
test_struct.iris_series.quarterly_mvts.UserData.third_comment = "Hello world!";
test_struct.iris_series.quarterly_mvts.UserData.second_cell = {'Hello', 'World'};
test_struct.iris_series.quarterly_mvts.UserData.second_struct = struct('thisfield', {'Hello', 'World'});
test_struct.iris_series.quarterly_mvts.UserData.some_number = 42;

de = DEFile(testFile, 'iris_colnames_field', 'Columns');
de.truncate();
de.write(test_struct);
de.close();


de = DEFile(testFile, 'read_to_iris',true);
results_struct = de.read();


for f = fieldnames(test_struct)'
    num_passed = 0;
    num_tests = length(fieldnames(test_struct.(f{1})));
    sub_struct_orig = test_struct.(f{1});
    sub_struct_results = results_struct.(f{1});
    for ff = fieldnames(sub_struct_orig)'
        orig = sub_struct_orig.(ff{1});
        read = sub_struct_results.(ff{1});
        same = compare_values(orig, read, ff{1}, true);
        num_passed = num_passed + same;
    end
    if num_passed == num_tests
        fprintf('✅ %s reading and writing is working.\n', f{1});
    else
        fprintf('❌ %d TESTS FAILED!\n', num_tests - num_passed);
    end
end

% Clean up
if exist(testFile, 'file')
    delete(testFile);
end