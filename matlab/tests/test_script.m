% addpath("DataEcon/matlab");
% 
% daec = DAEC()
% 
% import DAEC.*;


DAEC.load()
test_dir = '';
testFile = fullfile(test_dir, ['test_daec' num2str(randi(1000)) '.daec'])


test_struct = struct();

% scalars
test_struct.scalars = struct();
test_struct.scalars.double_val = 3.14159;
test_struct.scalars.empty_string = '';
test_struct.scalars.int64_val = int64(-9223372036854775808);
test_struct.scalars.large_val = 1.0e15;
test_struct.scalars.negative_val = -123.456;
test_struct.scalars.small_val = 1.0e-15;
test_struct.scalars.char_val = 'Hello World!';
test_struct.scalars.string_val = "Hello World!";
test_struct.scalars.zero_val = 0.0;
test_struct.scalars.true_val = true;
test_struct.scalars.false_val = false;
test_struct.scalars.complex_val = complex(2.5, 3.7);
test_struct.scalars.date_val1 = DEDate('quarterly', 2022,3);
test_struct.scalars.date_val2 = DEDate('monthly', 2022,9);
test_struct.scalars.date_val3 = DEDate('daily', 2022,9,10);
test_struct.scalars.date_val3 = DEDate('halfyearly', 2022,1);

% vectors
test_struct.vectors.double_vec = [1.1,  2.2,  3.3,  4.4,  5.5];
test_struct.vectors.int64_vec = int64([10,  -20,  30,  -40,  50]);
test_struct.vectors.large_vec = randn(1000,1);
test_struct.vectors.negative_vec = [-1.0,  -2.0,  -3.0,  -4.0,  -5.0];
test_struct.vectors.single_element = [-42.0];
test_struct.vectors.uint64_vec = uint64([10,  20,  30,  40,  50]);
test_struct.vectors.logical_vec = logical([true, false, false, true, true]);
test_struct.vectors.complex_vec1 = [1.1 + 5.5i,  2.2 + 4.4i,  3.3 + 3.3i,  4.4 + 2.2i,  5.5 + 1.1i];
test_struct.vectors.complex_vec2 = [1.1 + 5.5i,  2.2 + 4.4i,  3.3 + 3.3i,  4.4 + 2.2i,  5.5 + 1.1i]';

% matrices
test_struct.matrices.double_mat = [1.1,  2.2,  3.3,  4.4,  5.5; 1.2,  2.3,  3.4,  4.5,  5.6];
test_struct.matrices.int64_mat = int64([10,  -20,  30,  -40,  50; -10,  20,  -30,  40,  -50]);
test_struct.matrices.large_mat = randn(1000,800);
test_struct.matrices.negative_mat = [-1.0,  -2.0,  -3.0,  -4.0,  -5.0; -1.2,  -2.2,  -3.2,  -4.3,  -5.2];
test_struct.matrices.uint64_mat = uint64([10,  20,  30,  40,  50; 1010,  2020,  3030,  4040,  5050]);
test_struct.matrices.logical_mat = logical([true, false, false, true, true; false, true, true, false, false]);
test_struct.matrices.complex_mat = complex(randn(1000,800), randn(1000,800));

% tseries
% test_struct.tseries.quarterly_ts = TSeries(DEDate('quarterly', 2022,3), [1.1,  2.2,  3.3,  4.4,  5.5]);
% test_struct.tseries.monthly_ts = TSeries(DEDate('monthly', 2022,3), [10.1,  20.2,  30.3,  40.4,  50.5]);
% test_struct.tseries.yearly_ts = TSeries(DEDate('yearly', 2022), [100.1,  200.2,  300.3,  400.4,  500.5]);
test_struct.tseries.quarterly_ts = DESeries(DEAxis(DEDate('quarterly', 2022, 3), 5), [1.1,  2.2,  3.3,  4.4,  5.5]');
test_struct.tseries.monthly_ts = DESeries(DEAxis(DEDate('monthly', 2022, 3), 5), [10.1,  20.2,  30.3,  40.4,  50.5]');
test_struct.tseries.yearly_ts = DESeries(DEAxis(DEDate('yearly', 2022), 5), [100.1,  200.2,  300.3,  400.4,  500.5]');
test_struct.tseries.halfyearly_ts = DESeries(DEAxis(DEDate('halfyearly', 2022, 1), 5), [1000.1,  2000.2,  3000.3,  4000.4,  5000.5]');

% mvtseries
% test_struct.mvtseries.quarterly_mvts = MVTSeries(DEDate('quarterly', 2022,3), {'GDP', 'CPI'}, [1.1,  2.2,  3.3,  4.4,  5.5; 1.2,  2.3,  3.4,  4.5,  5.6]');
% test_struct.mvtseries.monthly_mvts = MVTSeries(DEDate('monthly', 2022,3), {'GDP', 'CPI'}, [10.1,  20.2,  30.3,  40.4,  50.5; 10.2,  20.3,  30.4,  40.5,  50.6]');
% test_struct.mvtseries.yearly_mvts = MVTSeries(DEDate('yearly', 2022), {'GDP', 'CPI'}, [100.1,  200.2,  300.3,  400.4,  500.5; 100.2,  200.3,  300.4,  400.5,  500.6]');

test_struct.mvtseries.quarterly_mvts = DESeries([DEAxis(DEDate('quarterly', 2022, 3), 5), DEAxis({'GDP', 'CPI'})], [1.1,  2.2,  3.3,  4.4,  5.5; 1.2,  2.3,  3.4,  4.5,  5.6]');
test_struct.mvtseries.complex_mvts = DESeries([DEAxis(DEDate('quarterly', 2022, 3), 5), DEAxis({'GDP', 'CPI'})], [1.1 + 5.5i,  2.2  + 4.4i,  3.3  + 3.3i,  4.4  + 2.2i,  5.5 + 1.1i; 1.2 + 2.1i,  2.3 + 3.2i,  3.4 + 4.3i,  4.5 + 5.4i,  5.6 + 6.5i]');
test_struct.mvtseries.monthly_mvts = DESeries([DEAxis(DEDate('monthly', 2022, 3), 5), DEAxis({'GDP', 'CPI'})], [10.1,  20.2,  30.3,  40.4,  50.5; 10.2,  20.3,  30.4,  40.5,  50.6]');
test_struct.mvtseries.yearly_mvts = DESeries([DEAxis(DEDate('yearly', 2022), 5), DEAxis({'GDP', 'CPI'})], [100.1,  200.2,  300.3,  400.4,  500.5; 100.2,  200.3,  300.4,  400.5,  500.6]');
test_struct.mvtseries.quarterly_mvts_large = DESeries([DEAxis(DEDate('quarterly', 2022,1), 50), DEAxis({'GDP', 'CONSUMPTION','INVESTMENT','RESIDENTIAL','INVENTORIES','GOVERNMENT','IMPORTS','EXPORTS','CPI','INTEREST_RATE','OUTPUT_GAP'})], randn(50,11));
test_struct.mvtseries.quarterly_mvts_verylarge = DESeries([DEAxis(DEDate('quarterly', 2022,1), 250), DEAxis({'GDP', 'CONSUMPTION','INVESTMENT','RESIDENTIAL','INVENTORIES','GOVERNMENT','IMPORTS','EXPORTS','CPI','INTEREST_RATE','OUTPUT_GAP','DISPINC','WAGES','PRINCIPAL_PAYMENTS','POTENTIAL','SOMETHING_A','SOMETHING_B','SOMETHING_C','SOMETHING_D','SOMETHING_E'})], randn(250,20));


test_struct.ndtseries.quarterly_ndts = DESeries([DEAxis({'GDP', 'CPI'}),DEAxis(DEDate('quarterly', 2022, 3), 5)], [1.1,  2.2,  3.3,  4.4,  5.5; 1.2,  2.3,  3.4,  4.5,  5.6]);
test_struct.ndtseries.monthly_ndts = DESeries([DEAxis({'GDP', 'CPI'}),DEAxis(DEDate('monthly', 2022, 3), 5)], [10.1,  20.2,  30.3,  40.4,  50.5; 10.2,  20.3,  30.4,  40.5,  50.6]);
test_struct.ndtseries.yearly_ndts = DESeries([DEAxis({'GDP', 'CPI'}),DEAxis(DEDate('yearly', 2022), 5)], [100.1,  200.2,  300.3,  400.4,  500.5; 100.2,  200.3,  300.4,  400.5,  500.6]);
nd_array = [1.1,  2.2,  3.3,  4.4,  5.5; 1.2,  2.3,  3.4,  4.5,  5.6];
nd_array(:,:,1) = [1.1,  2.1,  3.1,  4.1,  5.1; 1.2, 1.2, 1.3, 1.4, 1.5];
nd_array(:,:,2) = [1.2,  2.2,  3.2,  4.2,  5.2; 2.2, 2.2, 2.3, 2.4, 2.5];
nd_array(:,:,3) = [1.3,  2.3,  3.3,  4.3,  5.3; 3.2, 3.2, 3.3, 3.4, 3.5];
nd_array(:,:,4) = [1.4,  2.4,  3.4,  4.4,  5.4; 4.2, 4.2, 4.3, 4.4, 4.5];
nd_array(:,:,5) = [1.5,  2.5,  3.5,  4.5,  5.5; 5.2, 5.2, 5.3, 5.4, 5.5];
test_struct.ndtseries.three_level_ndts = DESeries([DEAxis({'GDP', 'CPI'}),DEAxis(DEDate('yearly', 2022), 5), DEAxis(DEDate('quarterly', 2022, 3), 5)], nd_array);

nd_complex = [1.1 + 1.0i,  2.2 + 1.2i,  3.3 + 1.3i,  4.4 + 1.4i,  5.5 + 1.5i; 1.2 + 1.6i,  2.3 + 1.7i,  3.4 + 1.8i,  4.5 + 1.9i,  5.6 + 1.10i];
nd_complex(:,:,1) = [1.1 + 1.11i,  2.1 + 1.12i,  3.1 + 1.13i,  4.1 + 1.14i,  5.1 + 1.15i; 1.2 + 1.16i, 1.2 + 1.17i, 1.3 + 1.18i, 1.4 + 1.19i, 1.5 + 1.20i];
nd_complex(:,:,2) = [1.2 + 1.21i,  2.2 + 1.22i,  3.2 + 1.23i,  4.2 + 1.24i,  5.2 + 1.25i; 2.2 + 1.26i, 2.2 + 1.27i, 2.3 + 1.28i, 2.4 + 1.29i, 2.5 + 1.30i];
nd_complex(:,:,3) = [1.3 + 1.31i,  2.3 + 1.32i,  3.3 + 1.33i,  4.3 + 1.34i,  5.3 + 1.35i; 3.2 + 1.36i, 3.2 + 1.37i, 3.3 + 1.38i, 3.4 + 1.39i, 3.5 + 1.40i];
nd_complex(:,:,4) = [1.4 + 1.41i,  2.4 + 1.42i,  3.4 + 1.43i,  4.4 + 1.44i,  5.4 + 1.45i; 4.2 + 1.46i, 4.2 + 1.47i, 4.3 + 1.48i, 4.4 + 1.49i, 4.5 + 1.50i];
nd_complex(:,:,5) = [1.5 + 1.51i,  2.5 + 1.52i,  3.5 + 1.53i,  4.5 + 1.54i,  5.5 + 1.55i; 5.2 + 1.56i, 5.2 + 1.57i, 5.3 + 1.58i, 5.4 + 1.59i, 5.5 + 1.60i];
test_struct.ndtseries.three_level_complex = DESeries([DEAxis({'GDP', 'CPI'}),DEAxis(DEDate('yearly', 2022), 5), DEAxis(DEDate('quarterly', 2022, 3), 5)], nd_complex);

test_struct.ndtseries.three_level_matrix = nd_array;


% other DESeries
test_struct.otherseries.named_1d = DESeries([DEAxis({'GDP', 'CPI'})], [1.1,  2.2]');
% test_struct.otherseries.named_1d_row = DESeries([DEAxis({'GDP', 'CPI'})], [1.1,  2.2]);
% test_struct.otherseries.plain_1d = DESeries([DEAxis(1:3)], [1.1,  2.2, 3.3]');
test_struct.otherseries.named_2d = DESeries([DEAxis({'GDP', 'CPI'}), DEAxis({'Yes', 'No'})], randn(2,2));
% test_struct.otherseries.plain_2d = DESeries([DEAxis(1:3), DEAxis(2:5)], randn(3,4));
test_struct.otherseries.quarterly_monthly = DESeries([DEAxis(DEDate('quarterly', 2022, 3), 2)], [DEDate('monthly',2022,9),  DEDate('monthly',2022,12)]');
test_struct.mvtseries.quarterly_monthly_mvts = DESeries([DEAxis(DEDate('quarterly', 2022, 3), 5), DEAxis({'GDP', 'CPI'})], [DEDate('monthly',2022,1),  DEDate('monthly',2022,2),  DEDate('monthly',2022,3),  DEDate('monthly',2022,4),  DEDate('monthly',2022,5); DEDate('monthly',2023,7),  DEDate('monthly',2023,8),  DEDate('monthly',2023,9),  DEDate('monthly',2024,10),  DEDate('monthly',2023,11)]');



de = DEFile(testFile);
de.truncate();
de.write(test_struct);
de.close();


de = DEFile(testFile);
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