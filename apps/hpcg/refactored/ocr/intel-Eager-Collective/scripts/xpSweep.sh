
OCR_INSTALL="$HOME/ocr_install_mpiicc_redevt-nodebug"

BASECPGN=${BASECPGN-"xp-benchmark-hpcgEagerRedevt-wrank"}

BATCH_TPL="hpcgEagerRedevt" TPLARG_QUEUE=XAS iter=50 TARGET_NAME=hpcgEagerRedevtReg CPGN=${BASECPGN}-regular NO_DEBUG=yes OCR_INSTALL=${OCR_INSTALL} CFLAGS="-DRANK_TIMER -DRANK_TIMER_ITONE" ./scripts/experiments.sh $@
BATCH_TPL="hpcgEagerRedevt" TPLARG_QUEUE=XAS iter=50 TARGET_NAME=hpcgEagerRedevtNoRed CPGN=${BASECPGN}-noreduction NO_DEBUG=yes OCR_INSTALL=${OCR_INSTALL} CFLAGS="-DRANK_TIMER -DRANK_TIMER_ITONE -DNO_REDUCTION" ./scripts/experiments.sh $@
BATCH_TPL="hpcgEagerRedevt" TPLARG_QUEUE=XAS iter=50 TARGET_NAME=hpcgEagerRedevtNoRedNoMg CPGN=${BASECPGN}-noreduction-nomg NO_DEBUG=yes OCR_INSTALL=${OCR_INSTALL} CFLAGS="-DRANK_TIMER -DRANK_TIMER_ITONE -DNO_REDUCTION -DNO_MG" ./scripts/experiments.sh $@
BATCH_TPL="hpcgEagerRedevt" TPLARG_QUEUE=XAS iter=50 TARGET_NAME=hpcgEagerRedevtNoMgNoHalo CPGN=${BASECPGN}-nomg-nohalo NO_DEBUG=yes OCR_INSTALL=${OCR_INSTALL} CFLAGS="-DRANK_TIMER -DRANK_TIMER_ITONE -DNO_MG -DNO_HALO_HPCG" ./scripts/experiments.sh $@
BATCH_TPL="hpcgEagerRedevt" TPLARG_QUEUE=XAS iter=50 TARGET_NAME=hpcgEagerRedevtNoRedNoMgNoHalo CPGN=${BASECPGN}-noreduction-nomg-nohalo NO_DEBUG=yes OCR_INSTALL=${OCR_INSTALL} CFLAGS="-DRANK_TIMER -DRANK_TIMER_ITONE -DNO_REDUCTION -DNO_MG -DNO_HALO_HPCG" ./scripts/experiments.sh $@
