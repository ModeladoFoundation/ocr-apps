# Only execute from the installation directory
if [ -f setup_env.sh ] && [ -f ./bin/cncocr_t ]; then
    if [ -d "$XSTACK_ROOT" ]; then
        echo 'Using existing $XSTACK_ROOT'
    elif [ -d "$OCR_INSTALL_ROOT" ]; then
        echo 'Using existing $OCR_INSTALL_ROOT'
    elif [ "$(basename $(dirname $PWD))" = hll ]; then
        export XSTACK_ROOT=$(dirname $(dirname $PWD))
        echo 'Set $XSTACK_ROOT'
    else
        cat <<EOF
Your CnC-OCR installation is not part of an XSTACK installation.
You will need to manually configure some environment variables.
You should set \$OCR_INSTALL_ROOT to point to your OCR installation directory.
Alternatively, see the CnC-OCR installation documentation for the expected directory structure.
EOF
    fi

    export CNCOCR_ROOT=$PWD
    echo 'Set $CNCOCR_ROOT'

    export PATH=$CNCOCR_ROOT/bin:$PATH
    echo 'Updated $PATH'
    echo 'Setup complete.'
else
    echo 'ERROR! Must source setup_env.sh from the CnC-OCR installation directory.'
fi
