# OCR environment
export OCR_HOME=$HOME/xstack/ocr
export OCR_INSTALL=$OCR_HOME/install/x86-pthread-x86

export PATH=$OCR_INSTALL/bin:$PATH
export LD_LIBRARY_PATH=$OCR_INSTALL/lib:$LD_LIBRARY_PATH

# R-Stream environment
export RSTREAM_HOME=/opt/reservoir/latest-thenretty/rstream-3.3.3
export ROCR_HOME=$RSTREAM_HOME/runtime/codelet/ocr
