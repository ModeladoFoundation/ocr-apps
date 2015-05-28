cp cgShim.c cg.c
touch cg.c
OCR_TYPE=x86  make clean run > outShim
OCR_TYPE=x86-mpi  make clean run >> outShim
cp cgNoShim.c cg.c
touch cg.c
OCR_TYPE=x86  make clean run > outNoShim
OCR_TYPE=x86-mpi  make clean run >> outNoShim
cp pcg.c cg.c
touch cg.c
OCR_TYPE=x86  make clean run > outPcg
OCR_TYPE=x86-mpi  make clean run >> outPcg
rm cg.c
