export RUN_TOOL=/usr/bin/time
#CONFIG_NUM_THREADS=1 OCR_TYPE=x86  WORKLOAD_ARGS="20 3 2" make clean uninstall run
OCR_TYPE=x86  WORKLOAD_ARGS="20 3 2" make clean uninstall run
OCR_TYPE=x86-mpi  WORKLOAD_ARGS="20 3 2" make clean uninstall run
#OCR_TYPE=x86  CONFIG_NUM_THREADS=1 make clean uninstall run
#OCR_TYPE=x86 make clean uninstall run
#OCR_TYPE=x86-mpi make clean run
