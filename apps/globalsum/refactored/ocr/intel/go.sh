OCR_TYPE=x86 CODE_TYPE=1 make clean run > outShim
OCR_TYPE=x86-mpi CODE_TYPE=1 make clean run >> outShim
OCR_TYPE=x86 CODE_TYPE=2 make clean run > outNoShim
OCR_TYPE=x86-mpi CODE_TYPE=2 make clean run >> outNoShim
OCR_TYPE=x86 CODE_TYPE=3 make clean run > outPcg
OCR_TYPE=x86-mpi CODE_TYPE=3 make clean run >> outPcg
