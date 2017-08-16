#!/bin/sh

CPGN=$1
BINARY_PATH=$2
DISPLAY_RESULT=$3

if [[ -z "${CPGN}" ]]; then
    echo "error: CPGN not defined"
    exit 1
fi

if [[ -z "${BINARY_PATH}" ]]; then
	BINARY_PATH="./install/x86-mpi/stencil2d_EagerStatic"
    echo "warning: BINARY_PATH not provided, default to ${BINARY_PATH}"
fi

function replaceAddrWithName() {
    file=$1
    binary=$2

    #TODO: mangle with binary name
    funcRepo=elf_tmp

    if [[ ! -f ${funcRepo} ]]; then
        readelf -a $binary | grep FUNC | sed -e "s/ \+/ /g"> ${funcRepo}2
        while read -r line; do
            ptr=`echo $line | cut -d' ' -f2-2 | uniq`
            name=`echo $line | cut -d' ' -f8-8 | uniq`
            echo "$ptr $name"
        done < ${funcRepo}2 > ${funcRepo}
    fi
    FUNCS=`more ${file} | grep "action=func" | cut -d' ' -f5-5 | sort | uniq | sed -e "s/key=0x//g"`

    newFile=${file}_named
    cp $file ${newFile}

    for f in `echo $FUNCS`; do
        FUNC_NAME=`grep $f ${funcRepo} | cut -d' ' -f2-2 | uniq`
        #echo "ADDR=$f FUNC_NAME=$FUNC_NAME"
        if [[ -n "$FUNC_NAME" ]]; then
            more ${newFile} | sed -e "s/0x$f/$FUNC_NAME/g" > tmp_file2
            mv tmp_file2 ${newFile}
        fi
    done
    if [[ -n "${DISPLAY_RESULT}" ]]; then
        more ${file}_named | grep "^\[" | sort
    else
        echo "output generated in: ${newFile}"
    fi
}

if [[ -f "${CPGN}" ]]; then
    replaceAddrWithName ${CPGN} ${BINARY_PATH}
elif [[ -d "${CPGN}" ]]; then
    for file in `grep --files-with-matches -R "\[EDT\] | key=" ${CPGN}`; do
        replaceAddrWithName ${file} ${BINARY_PATH}
    done
else
    echo "error: $CPGN path is neither a file nor a directory"
fi
