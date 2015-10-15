#OCR_TYPE=x86  make clean
#OCR_TYPE=x86  CODE_TYPE=4 make run 10 50 10000
#OCR_TYPE=x86  make clean
#OCR_TYPE=x86  CODE_TYPE=5 make run 10 50 10000
#OCR_TYPE=x86  make clean
#OCR_TYPE=x86  CODE_TYPE=6 make run 10 50 10000
OCR_TYPE=x86-mpi make clean
OCR_TYPE=x86-mpi CODE_TYPE=4 make run 10 50 10000
