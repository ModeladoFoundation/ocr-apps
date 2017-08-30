rm -fr build/ install/

if [ $# -gt 5 ]; then
    echo "usage: ./go.sh [dimension] [maxRefLvl] [timesteps] [refineFreq]"

else
    #echo "Running miniAMR using nodefile. To change, edit go.sh."
    #OCR_NODEFILE="/home/bpawlow/freshMiniAMR/xstack/apps/apps/miniAMR/refactored/ocr/intel-bryan/nodefile" OCR_NUMNODES=4 OCR_TYPE=x86-mpi make run arg1=$1 arg2=$2 arg3=$3 arg4=$4 arg5="YES_COARSEN"
    echo "Running miniAMR using only OCR_NUMNODES. To change, edit go.sh."
    OCR_NUMNODES=4 OCR_TYPE=x86-mpi make run arg1=$1 arg2=$2 arg3=$3 arg4=$4 arg5="YES_COARSEN"
    #echo "Running miniAMR using single node shared memory. To change, edit go.sh"
    #OCRRUN_GDB=yes OCR_TYPE=x86 make run arg1=$1 arg2=$2 arg3=$3
    #OCR_TYPE=x86 make run arg1=$1 arg2=$2 arg3=$3 arg4=$4 arg5="YES_COARSEN"
fi
