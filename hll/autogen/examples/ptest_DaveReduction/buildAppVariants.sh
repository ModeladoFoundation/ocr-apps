#!/bin/bash

# clean reduction
function cleanLibs {
    rm ../../../../apps/libs/src/reduction/x86/*.a ../../../../apps/libs/src/reduction/x86/*.o ../../../../apps/libs/install/x86/lib/libreduction.a
}

# Build libraries and reduction driver
function buildTest {
    OCR_TYPE=x86-mpi make uninstall clean
    make V=1
    make install V=1
}

cleanLibs
export TARGET="z_ptest_daveReduction_A2_F1"
export APP_CFLAGS="-DARITY=2 -DCOUNT_FOR_FORKJOIN=1"
buildTest

