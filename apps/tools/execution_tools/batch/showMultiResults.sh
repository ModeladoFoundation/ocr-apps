#!/bin/bash

#
# Extract scaling and efficiency for a given 'core' count
# over multiple runs involving varying number of nodes.
#

if [ "$#" -ne 3 ]; then
    echo "usage: showMultiResults.sh 'folder' 'cores' "
    echo "    folder :  where to look for the run output"
    echo "    cores  :  a list of the number of cores to look for in the run output"
    echo "    mode   :  node or 'pes_comp' (all computation workers)"
    exit 1
fi

folder="$1"
cores="$2"
mode="$3"
metric=${metric-"duration"}
filenamePrefix=${filenamePrefix-"ws_Stencil2D"}
filename=${filenamePrefix}

# Extract number of nodes from file name.
# nodes=`grep -l Throughput ${folder}/* | cut -d'.' -f3-3 | sed s/N//g | sort -g`
# to dev/null to avoid errors with folders
nodes=`grep -l Throughput ${folder}/* 2>/dev/null`
acc=""
for n in `echo $nodes`; do
    filename=${n##*/}
    count=`echo $filename | cut -d'.' -f2-2 | sed s/N//g`
    acc+=" $count"
     # | cut -d'/' cut -d'.' -f2-2 | sed s/N//g | sort -g`
done
nodes=`echo $acc | fmt -1 | sort -g`
nodesfmt=`echo $nodes | tr '\n' ' '`
# echo "Nodes found => ${nodesfmt}"

# Find the reports
reports=""
for r in `echo ${nodes}`; do
    # Look for the report name the runlog file
    reportfile=`grep "/report" ${folder}/${filenamePrefix}*.${r}N* | cut -d' ' -f5-5`
    # If not found, look for any report-* file. This can happen if the job crashed or
    # got interrupted and the report were generated post-mortem to use whatever data is available
    if [[ -z "${reportfile}" ]]; then
        reportfile=`find ${folder} -name "report-*"`
    fi
    reports="$reports ${reportfile}"
done
# echo "Reports found => ${reports}"

# Take into account NUMA variants
NUMA_VARIANT=${NUMA_VARIANT-1}

function adaptCoreCountToNUMA() {
    local coreList="$1"
    local newCoreList=""
    for core in `echo $coreList`; do
        compCore=$(( ${core} - 1 ))
        newCompCore=$(( ${compCore} * ${NUMA_VARIANT} ))
        newCore=$(( ${newCompCore} + ${NUMA_VARIANT} ))
        newCoreList="${newCoreList}${newCore} "
    done
    echo "$newCoreList" | sed -e "s/ $//g"
}

# the core count to look for in each node run
nodesArray=($nodes)

# echo "reports=${reports}"

if [[ "${METRIC}" == "flops" ]]; then
    TARGET_FIELD=2
    TARGET_NAME="MFlop/s"
else
    TARGET_FIELD=6
    TARGET_NAME="Time"
fi

let ii=0;
for c in `echo $cores`; do
    let i=0;
    corePerNode=`adaptCoreCountToNUMA "$c"`
    for mreport in `echo ${reports}`; do
        nodeCount=${nodesArray[${i}]}
        if [[ "$mode" == "node" ]]; then
            #Single Computation PE per node
            workerCount=${nodeCount}
        elif [[ "$mode" == "pes_comp" ]]; then
            #All computation PEs (node * comp PEs)
            workerCount=$(( $nodeCount * (${c}-1) * ${NUMA_VARIANT} ))
        else
            echo "error: unknown mode"
            exit 1
        fi
        IFS=$'\n'
        line=`grep "^${c} " ${mreport}`
        cleanLine=`echo ${line} | sed -e "s/ \+/ /g"`
        flops=`echo ${cleanLine} | cut -d' ' -f ${TARGET_FIELD}-${TARGET_FIELD}`

        if [[ ${ii} == 0 ]]; then
            workerCountBase0=$((${workerCount}))
            flopsBase0=$flops
        fi
        if [[ ${i} == 0 ]]; then
            flops0=$flops
            workerCount0=$((${workerCount}))
            echo "Nodes Cores/node Comp-cores ${TARGET_NAME} Scaling(base) Efficiency(base) Scaling Efficiency NbRuns Stddev"
        fi
        if [[ "${METRIC}" == "flops" ]]; then
            isZero=`echo "scale=3; $flopsBase0 == 0" | bc -l`
            if [[ "${isZero}" -eq "1" ]]; then
                scalingBase=0
            else
                scalingBase=`echo "scale=3; $flops/$flopsBase0" | bc -l`
            fi
        else
            isZero=`echo "scale=3; $flops == 0" | bc -l`
            if [[ "${isZero}" -eq "1" ]]; then
                scalingBase=0
            else
                scalingBase=`echo "scale=3; $flopsBase0/$flops" | bc -l`
            fi
        fi
        efficiencyBase=`echo "scale=3; (${scalingBase}*${workerCountBase0})/${workerCount}" | bc -l`
        if [[ "${METRIC}" == "flops0" ]]; then
            isZero=`echo "scale=3; $flops0 == 0" | bc -l`
            if [[ "${isZero}" -eq "1" ]]; then
                scaling=`echo "scale=3; $flops/$flops0" | bc -l`
            else
                scaling=0
            fi
        else
            isZero=`echo "scale=3; $flops == 0" | bc -l`
            if [[ "${isZero}" -eq "1" ]]; then
                scaling=0
            else
                scaling=`echo "scale=3; $flops0/$flops" | bc -l`
            fi
        fi
        efficiency=`echo "scale=3; (${scaling}*${workerCount0})/${workerCount}" | bc -l`
        nbRuns=`echo ${cleanLine} | cut -d' ' -f 4-4`
        if [[ "${METRIC}" == "flops" ]]; then
            stddev=`echo ${cleanLine} | cut -d' ' -f 3-3`
        else
            stddev=`echo ${cleanLine} | cut -d' ' -f 7-7`
        fi
        echo "${nodeCount} ${corePerNode}  ${workerCount} ${flops} ${scalingBase} ${efficiencyBase} ${scaling} ${efficiency} ${nbRuns} ${stddev}"
        let i=$i+1
        let ii=${ii}+1
        IFS=$' '
    done
done | column -t

