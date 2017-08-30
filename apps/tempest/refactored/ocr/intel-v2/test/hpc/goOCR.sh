WORK_DIR=$PWD
cd ../../
make
cd $WORK_DIR
rm -fr ./build/x86 ./install/x86
OCR_TYPE=x86 make -f MakeOCR uninstall run
