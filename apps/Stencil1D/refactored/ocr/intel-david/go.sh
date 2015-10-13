OCR_TYPE=x86  CODE_TYPE=1 make clean run > outSticky
OCR_TYPE=x86-mpi  CODE_TYPE=1 make clean run >> outSticky
OCR_TYPE=x86  CODE_TYPE=2 make clean run > outGuid
OCR_TYPE=x86-mpi  CODE_TYPE=2 make clean run >> outGuid
OCR_TYPE=x86  CODE_TYPE=3 make clean run > outOnce
OCR_TYPE=x86-mpi  CODE_TYPE=3 make clean run >> outOnce
OCR_TYPE=x86  CODE_TYPE=4 make clean run > outStickyLG
OCR_TYPE=x86-mpi  CODE_TYPE=4 make clean run >> outStickyLG
OCR_TYPE=x86  CODE_TYPE=5 make clean run > outOncePI
OCR_TYPE=x86-mpi  CODE_TYPE=5 make clean run >> outOncePI
OCR_TYPE=x86  CODE_TYPE=6 make clean run > outGuidPI
OCR_TYPE=x86-mpi  CODE_TYPE=6 make clean run >> outGuidPI

