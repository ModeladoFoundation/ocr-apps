# Regression infrastructure uses basevalues supplied by app developer
# to normalize trend plots
# Add entry for each test -> 'test_name':value_to_normalize_against
# test_name should be same as run job name
# If you don't desire normalization , make an entry and pass
# 1 as value. If you are adding a new app , find avg. runtime by
# running app locally and use it.

test_baseval={
                'time_ocr-run-kernel-cholesky-x86-remote-regression':0.381,
                'scale_ocr-run-kernel-cholesky-x86-remote-scaling':1.0,
                'time_ocr-run-kernel-fft-x86-remote-regression':0.893,
                'scale_ocr-run-kernel-fft-x86-remote-scaling':1.0,
                'time_ocr-run-kernel-fibonacci-x86-remote-regression':0.083,
                'scale_ocr-run-kernel-fibonacci-x86-remote-scaling':1,
                'time_ocr-run-kernel-smithwaterman-x86-remote-regression':0.047,
                'scale_ocr-run-kernel-smithwaterman-x86-remote-scaling':1,
                'time_ocr-run-kernel-p2p-x86-remote-regression':0.252,
                'scale_ocr-run-kernel-p2p-x86-remote-scaling':1,
                'time_ocr-run-kernel-globalsum-x86-remote-regression':0.029,
                'scale_ocr-run-kernel-globalsum-x86-remote-scaling':1,
                'time_ocr-run-kernel-triangle-x86-remote-regression':33.983,
                'scale_ocr-run-kernel-triangle-x86-remote-scaling':1,
                'time_ocr-run-kernel-Stencil1DDavid-x86-remote-regression':0.186,
                'scale_ocr-run-kernel-Stencil1DDavid-x86-remote-scaling':1,
                'time_ocr-run-kernel-Stencil1DChandra-x86-remote-regression':0.942,
                'scale_ocr-run-kernel-Stencil1DChandra-x86-remote-scaling':1,
                'time_ocr-run-kernel-Stencil1Dlite-x86-remote-regression':1.1469,
                'scale_ocr-run-kernel-Stencil1Dlite-x86-remote-scaling':1,
                'time_ocr-run-kernel-comdsdsc-x86-remote-regression':0.106,
                'scale_ocr-run-kernel-comdsdsc-x86-remote-scaling':1,
                'time_ocr-run-kernel-comdlite-x86-remote-regression':27.279,
                'time_ocr-run-kernel-hpgmgsdsc-x86-remote-regression':0.043,
                'scale_ocr-run-kernel-hpgmgsdsc-x86-remote-scaling':1,
                'time_ocr-run-kernel-hpgmglite-x86-remote-regression':10.963,
                'scale_ocr-run-kernel-hpgmglite-x86-remote-scaling':1,
                'time_ocr-run-kernel-tempestsw2lite-x86-remote-regression':0.368,
                'scale_ocr-run-kernel-tempestsw2lite-x86-remote-scaling':1,
                'time_ocr-run-kernel-xsbench-x86-remote-regression':2.904,
                'scale_ocr-run-kernel-xsbench-x86-remote-scaling':1,
                'time_ocr-run-kernel-xsbenchlite-x86-remote-regression':0.967,
                'time_ocr-run-kernel-lulesh2pnnl-x86-remote-regression':0.032,
                'time_ocr-run-kernel-hpgmg-pnnl-x86-remote-regression':1.000,
                'time_ocr-run-kernel-SNAPlite-x86-remote-regression':0.861,
}
