export RUN_TOOL=/usr/bin/time
WORKLOAD_ARGS="3 4 5 16 50 0" OCR_TYPE=x86 make uninstall clean run
WORKLOAD_ARGS="3 4 5 16 50 0" OCR_TYPE=x86-mpi OCR_NUM_NODES=1  make uninstall clean run
#WORKLOAD_ARGS="3 4 5 16 50 0" OCR_TYPE=x86-mpi make uninstall clean run
