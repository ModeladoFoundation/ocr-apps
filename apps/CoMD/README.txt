baseline_x86/
    Pulled CoMDv1.1 (officially released) from the CoMD repository - https://github.com/exmatex/CoMD
    src-openmp --> Doesn't use force symmetry across the threads
    src-mpi --> Does use force symmetry within a rank

sdsc-mpi_openmp/
    Slightly modified implementation for OpenMP. Uses symmetric force computation across the threads.

cnc_v1.0/
    Intel CnC version implemented by Riyaz Haque (as part of 2014 ExMatEx Co-design Summer School, LLNL) with David Richards

ocr_v1.0/
    LLNL 2014 summer school implementation.

sdsc-ocr_d/
    OCR implementation from SDSC
