# Regression infrastructure uses basevalues supplied by app developer
# to normalize trend plots
# Add entry for each test -> 'test_name':value_to_normalize_against
# test_name should be same as run job name
# If you don't desire normalization , make an entry and pass
# 1 as value. If you are adding a new app , find avg. runtime by
# running app locally and use it.

test_baseval={
                'time_ocr-run-kernel-cholesky-x86-remote-regression':0.49,
                'scale_ocr-run-kernel-cholesky-x86-remote-scaling':1.0,
                'time_ocr-run-kernel-fft-x86-remote-regression':0.9,
                'scale_ocr-run-kernel-fft-x86-remote-scaling':1.0,
                'time_ocr-run-kernel-fibonacci-x86-remote-regression':0.91,
                'scale_ocr-run-kernel-fibonacci-x86-remote-scaling':1,
                'time_ocr-run-kernel-smithwaterman-x86-remote-regression':0.35,
                'scale_ocr-run-kernel-smithwaterman-x86-remote-scaling':1,
                'time_ocr-run-kernel-Stencil1DDavid-x86-remote-regression':0.65,
                'scale_ocr-run-kernel-Stencil1DDavid-x86-remote-scaling':1,
                'time_ocr-run-kernel-Stencil1DChandra-x86-remote-regression':8.0,
                'scale_ocr-run-kernel-Stencil1DChandra-x86-remote-scaling':1,
                'time_ocr-run-kernel-comdllnl-x86-remote-regression':14.4,
                'scale_ocr-run-kernel-comdllnl-x86-remote-scaling':1,
                'time_ocr-run-kernel-comdsdsc-x86-remote-regression':0.20,
                'scale_ocr-run-kernel-comdsdsc-x86-remote-scaling':1,
                'time_ocr-run-kernel-hpgmgsdsc-x86-remote-regression':0.80,
                'scale_ocr-run-kernel-hpgmgsdsc-x86-remote-scaling':1,
                'time_ocr-run-kernel-hpgmglite-x86-remote-regression':22.80,
                'scale_ocr-run-kernel-hpgmglite-x86-remote-scaling':1,
                'time_ocr-run-kernel-tempestsw2lite-x86-remote-regression':1.4,
                'scale_ocr-run-kernel-tempestsw2lite-x86-remote-scaling':1,
                'time_ocr-run-kernel-xsbench-x86-remote-regression':20.4,
                'scale_ocr-run-kernel-xsbench-x86-remote-scaling':1,
                'time_ocr-run-kernel-xsbenchlite-x86-remote-regression':20.4,
                'time_ocr-run-kernel-lulesh2pnnl-x86-remote-regression':0.1,
}
