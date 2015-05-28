cp cgShim.c cg.c
touch cg.c
OCR_TYPE=x86  make run > outShim
OCR_TYPE=x86-mpi  make run >> outShim
cp cgNoShim.c cg.c
touch cg.c
OCR_TYPE=x86  make run > outNoShim
OCR_TYPE=x86-mpi  make run >> outNoShim
cp pcg.c cg.c
touch cg.c
OCR_TYPE=x86  make run > outPcg
OCR_TYPE=x86-mpi  make run >> outPcg
rm cg.c
