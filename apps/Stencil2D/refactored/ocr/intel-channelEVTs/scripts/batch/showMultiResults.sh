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
filename="ws_Stencil2D"

# Extract number of nodes from file name.
# nodes=`grep -l Throughput ${folder}/* | cut -d'.' -f3-3 | sed s/N//g | sort -g`
nodes=`grep -l Throughput ${folder}/*`
acc=""
for n in `echo $nodes`; do
    filename=${n##*/}
    count=`echo $filename | cut -d'.' -f2-2 | sed s/N//g`
    acc+=" $count"
     # | cut -d'/' cut -d'.' -f2-2 | sed s/N//g | sort -g`
done
nodes=`echo $acc | fmt -1 | sort -g`
nodesfmt=`echo $nodes | tr '\n' ' '`
#echo "Nodes found => ${nodesfmt}"

# Find the reports
reports=""
for r in `echo ${nodes}`; do
    reportfile=`grep "/report" ${folder}/ws_Stencil2D*.${r}N* | cut -d' ' -f5-5`
    reports="$reports ${reportfile}"
done
# echo "Reports found => ${reports}"

# the core count to look for in each node run
nodesArray=($nodes)

#echo "reports=${reports}"

let ii=0;
for c in `echo $cores`; do
    let i=0;
    for mreport in `echo ${reports}`; do
        nodeCount=${nodesArray[${i}]}
        if [[ "$mode" == "node" ]]; then
            #Single Computation PE per node
            workerCount=${nodeCount}
        elif [[ "$mode" == "pes_comp" ]]; then
            #All computation PEs (node * comp PEs)
            workerCount=$(( $nodeCount *(${c}-1) ))
        else
            echo "error: unknown mode"
            exit 1
        fi
        IFS=$'\n'
        line=`grep "^${c} " ${mreport}`
        cleanLine=`echo ${line} | sed -e "s/ \+/ /g"`
        flops=`echo ${cleanLine} | cut -d' ' -f 2-2`

        if [[ ${ii} == 0 ]]; then
            workerCountBase0=$((${workerCount}))
            flopsBase0=$flops
        fi
        if [[ ${i} == 0 ]]; then
            flops0=$flops
            workerCount0=$((${workerCount}))
            echo "Nodes Cores/node Comp-cores MFlop/s Scaling(base) Efficiency(base) Scaling Efficiency NbRuns Stddev"
        fi
        scalingBase=`echo "scale=3; $flops/$flopsBase0" | bc -l`
        efficiencyBase=`echo "scale=3; (${scalingBase}*${workerCountBase0})/${workerCount}" | bc -l`
        scaling=`echo "scale=3; $flops/$flops0" | bc -l`
        efficiency=`echo "scale=3; (${scaling}*${workerCount0})/${workerCount}" | bc -l`
        nbRuns=`echo ${cleanLine} | cut -d' ' -f 4-4`
        stddev=`echo ${cleanLine} | cut -d' ' -f 3-3`
        echo "${nodeCount} ${c} ${workerCount} ${flops} ${scalingBase} ${efficiencyBase} ${scaling} ${efficiency} ${nbRuns} ${stddev}"
        let i=$i+1
        let ii=${ii}+1
        IFS=$' '
    done
done | column -t

