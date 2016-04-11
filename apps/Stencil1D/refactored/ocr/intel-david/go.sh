#export RUN_TOOL=valgrind
export RUN_TOOL=/usr/bin/time
OCR_TYPE=x86-mpi CODE_TYPE=1 make clean uninstall run 15 10 1500
OCR_TYPE=x86-mpi CODE_TYPE=2 make clean uninstall run 15 10 1500
OCR_TYPE=x86-mpi CODE_TYPE=3 make clean uninstall run 15 10 1500
OCR_TYPE=x86-mpi CODE_TYPE=4 make clean uninstall run 15 10 1500
OCR_TYPE=x86-mpi CODE_TYPE=5 make clean uninstall run 15 10 1500
OCR_TYPE=x86-mpi CODE_TYPE=6 make clean uninstall run 15 10 1500
OCR_TYPE=x86-mpi CODE_TYPE=7 make clean uninstall run 15 10 1500
#OCR_TYPE=x86-mpi   WORKLOAD_ARGS="15, 10, 1500" CODE_TYPE=1 make clean uninstall run
#OCR_TYPE=x86  WORKLOAD_ARGS="3, 10, 30" CODE_TYPE=1 CONFIG_NUM_THREADS=1 make clean uninstall run
#OCR_TYPE=x86-mpi  CODE_TYPE=1 make clean run >> outSticky
#OCR_TYPE=x86  CODE_TYPE=2 make clean run > outGuid
#OCR_TYPE=x86-mpi  CODE_TYPE=2 make clean run >> outGuid
#OCR_TYPE=x86  CODE_TYPE=3 make clean run > outOnce
#OCR_TYPE=x86-mpi  CODE_TYPE=3 make clean run >> outOnce
#OCR_TYPE=x86  CODE_TYPE=4 make clean run > outStickyLG
#OCR_TYPE=x86-mpi  CODE_TYPE=4 make clean run >> outStickyLG
#OCR_TYPE=x86  CODE_TYPE=5 make clean run > outOncePI
#OCR_TYPE=x86-mpi  CODE_TYPE=5 make clean run >> outOncePI
#OCR_TYPE=x86  CODE_TYPE=6 make clean run > outGuidPI
#OCR_TYPE=x86-mpi  CODE_TYPE=6 make clean run >> outGuidPI
