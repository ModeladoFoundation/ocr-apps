#!/bin/bash

#points to the directory where we want to run the job
#this is where output files and logs will be.
hme="/home/rsalmon/dhome/fastfw/run/test"

# points to where the binary executable is
# These scripts assume you have a binary already built and ready to go
# we assume you did a "make install" prior.
srchme="/home/rsalmon/dhome/fastfw/new/apps/apps/Stencil2D/refactored/mpilite/intel"
exehme="${srchme}/install/x86-mpi/stencil"

# Itac collecttion options. Milage my vary.
ITAC_LIST=(none p2p coll all)

# list of possible problem sizes.
psize_list=(small medium large)

tn=mpilite
MYITAC=${ITAC_LIST[0]}
psize=${psize_list[0]}

# change these to control number of nodes. These are in powers of two
#ns = start  = 2^ns   ne= end = 2^ne
#ns =1 , ns=7 would run jobs from 2 - 128 nodes 2,4,8,....64,128.
ns=1
ne=1

#Number of threads per node.
ph=18

# make sure this directory exists.
shme="${hme}/${psize}/${tn}-thornodes-${MYITAC}"  # ppn=18
if [ ! -d "${hme}/${psize}" ]; then
    mkdir -p "${hme}/${psize}"
fi

#submit strong scaling jobs
for i in `seq $ns $ne`; do
    scale="strong"
    rhm="${shme}-${scale}"
    if [ ! -d $rhm ]; then
        mkdir $rhm
    fi

    nodes=`echo 2^${i} |bc`
    np="`echo "${nodes}* ${ph}"|bc`"
    lname=$scale.np.$np.log
    sbatch --nodes=${nodes} --job-name="${lname}" --output="$rhm/${lname}" ${srchme}/experiments/x86/scripts/thor/simple-stencil2d-${tn}.sh ${scale} ${rhm} ${MYITAC} ${psize} ${exehme} ${ph}

done


# submit weak scaling jobs
for i in `seq $ns $ne`; do
    scale="weak"
    rhm="${shme}-${scale}"
    if [ ! -d $rhm ]; then
        mkdir $rhm
    fi

    nodes=`echo 2^${i} |bc`
    np="`echo "${nodes}* ${ph}"|bc`"
    lname=$scale.np.$np.log
    sbatch --nodes=${nodes} --job-name="${lname}" --output="$rhm/${lname}" ${srchme}/experiments/x86/scripts/thor/simple-stencil2d-${tn}.sh ${scale} ${rhm} ${MYITAC} ${psize} ${exehme} ${ph}
done


