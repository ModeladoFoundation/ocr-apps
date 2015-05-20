cp stencil1Dsticky.c stencil1D.c
touch stencil1D.c
OCR_TYPE=x86-pthread-x86  make run > outSticky
OCR_TYPE=x86-pthread-mpi  make run >> outSticky
cp stencil1Dguid.c stencil1D.c
touch stencil1D.c
OCR_TYPE=x86-pthread-x86  make run > outGuid
OCR_TYPE=x86-pthread-mpi  make run >> outGuid
cp stencil1Donce.c stencil1D.c
touch stencil1D.c
OCR_TYPE=x86-pthread-x86  make run > outOnce
OCR_TYPE=x86-pthread-mpi  make run >> outOnce
rm stencil1D.c
