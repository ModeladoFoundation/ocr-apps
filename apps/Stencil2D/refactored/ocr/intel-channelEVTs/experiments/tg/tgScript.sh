#!/bin/bash

sizes=(256 1024)
iters=(1 11)
BLOCKCOUNTS=(1 2 4)

server_nodes=(211 212 213)
client_nodes=(221 222 223 224) # 225 226 227 228 229 230 231)

fsimblockspernode=6
beginNode=0
snode=0

function generateMachineConfig()
{
    local blockcount=$1
    local servernode=$2

    local fsimblockcount=$((4+${blockcount}))
    local nodes=$(((${fsimblockcount}+${fsimblockspernode}-1)/${fsimblockspernode}))

    local file="machineConfig_${blockcount}"
    local fileKillNodes="killnodes_${blockcount}"

    rm $fileKillNodes
    rm $file

    echo $servernode > $fileKillNodes

cat << END_CAT > $file
[Machines]
    machine_count=${nodes}

END_CAT

    for(( inode=0; inode<=$nodes-1; inode++ )); do

        client=${client_nodes[$(($beginNode+$inode))]}
        name=`printf "thor-%03d-ib" $client`
        echo "[machine${inode}]" >> $file
        echo "    name=\"$name\"" >> $file
        echo "    max_blocks=${fsimblockspernode}" >> $file
        echo >> $file

        echo $name >> $fileKillNodes
    done

    echo $nodes $file
}

# strong-scaling

rm run_commands

for BLOCKCOUNT in ${BLOCKCOUNTS[@]}; do

    jfile=jobScript_${BLOCKCOUNT}
    rm ${jfile}.sh
    echo "#!/bin/bash" >> ${jfile}.sh
    echo >> ${jfile}.sh

    servernode=`printf "thor-%03d-ib" ${server_nodes[$snode]}`; snode=$(($snode+1))
    rvalues=(`generateMachineConfig $BLOCKCOUNT $servernode`)
    nodes=${rvalues[0]}
    machineConfigFile=${rvalues[1]}
    beginNode=$((${beginNode}+${nodes}))

    #echo $beginNode

    for size in ${sizes[@]}; do
    for iter in ${iters[@]}; do

        sed "s/block_count = BLOCKCOUNT/block_count   = ${BLOCKCOUNT}/g" config.tpl > config.cfg

        jobHeader="${size}_${iter}_${BLOCKCOUNT}"
        winstall="install_$jobHeader"

        jobHeader="${BLOCKCOUNT}"

        BUILD_CMD="MACHINE_CONFIG=$PWD/${machineConfigFile} WORKLOAD_INSTALL_ROOT=./${winstall} make -f Makefile.tg install WORKLOAD_ARGS='${size} $((8*$BLOCKCOUNT)) ${iter}'"
        eval $BUILD_CMD
        mkdir -p ./${winstall}/tg/logs
        #RUN_CMD="MACHINE_CONFIG=$PWD/${machineConfigFile} WORKLOAD_INSTALL_ROOT=./${winstall} make -f Makefile.tg run WORKLOAD_ARGS='${size} $((8*$BLOCKCOUNT)) ${iter}'"
        WDIR=`pwd`
        RUN_CMD="ENERGY=yes TG_INSTALL=$TG_TOP/tg/install WORKLOAD_INSTALL=$WDIR/${winstall}/tg $TG_TOP/tg/install/bin/fsim -s -L $WDIR/${winstall}/tg/logs -c $WDIR/${machineConfigFile} -c $WDIR/${winstall}/tg/config.cfg -c $TG_TOP/tg/install/fsim-configs/Energy.cfg -c $TG_TOP/tg/install/fsim-configs/dvfs-default.cfg"
        echo date >> ${jfile}.sh
        echo $RUN_CMD >> ${jfile}.sh
        echo date >> ${jfile}.sh

    done
    done

    chmod +x ${jfile}.sh

    echo "ssh ${servernode} \"source ~/.bash_profile; cd $PWD; ./${jfile}.sh &> ${jfile}.out &\"" >> run_commands

done
