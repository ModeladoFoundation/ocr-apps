rm -fr build/ install/

OCR_GDBRUN=yes OCR_NUM_NODES=4 OCR_TYPE=x86-mpi make run
#OCR_TYPE=x86 make run
