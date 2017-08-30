
# jobs are serialized with -N 63 in the batch file

#
# Driver to go over a set of experiments parameters
# and build, generate, run, show results
#

CMD_BUILD="-build"
CMD_GEN="-gen"

if [[ $# == 0 ]]; then
    CMDS=${CMDS-"-clean -build -gen -run"}
else
    CMDS="$*"
fi

export CORE_SCALING=${CORE_SCALING-"9"}

export NODE_SCALING=${NODE_SCALING-"1 2 4 8 16 32 64 128"}
export nxyz=${nxyz-"2_2_2 4_2_2 4_4_2 4_4_4 8_4_4 8_8_4 8_8_8 16_8_8"}

export size="64"
export iter=${iter-"50"}
export debug="0"

for CMD in `echo ${CMDS}`; do
    ./scripts/driver.sh ${CMD}
done

