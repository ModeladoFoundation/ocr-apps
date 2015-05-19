# Regression infrastructure uses basevalues supplied by app developer
# to normalize trend plots
# Add entry for each test -> 'test_name':value_to_normalize_against
# test_name should be same as run job name
# If you don't desire normalization , make an entry and pass
# 1 as value. If you are adding a new app , find avg. runtime by
# running app locally and use it.

test_baseval={
                'time_ocr-run-kernel-cholesky-x86-remote-regression':0.49,
                'time_ocr-run-kernel-fft-x86-remote-regression':0.9,
                'time_ocr-run-kernel-fibonacci-x86-remote-regression':0.91,
                'time_ocr-run-kernel-smithwaterman-x86-remote-regression':0.35,
                'time_ocr-run-kernel-fibonacci-x86-remote-scaling':1,
                'time_ocr-run-kernel-comdllnl-x86-remote-regression':14.4,
                'time_ocr-run-kernel-comdsdsc-x86-remote-regression':0.20,
                'time_ocr-run-kernel-hpgmglite-x86-remote-regression':21.8,
}
