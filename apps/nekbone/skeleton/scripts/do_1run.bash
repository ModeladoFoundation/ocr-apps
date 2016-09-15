#!/usr/bin/env bash

fullTemplateFilename='sizedata.template'
szDataFile='sizedata.h'

Usage () {
    echo "USAGE: $0 Rx Ry Rz Ex Ey Ez Pbegin Pend Pstep CGcount"
    echo "    (Rx,Ry,Rz) are the MPI ranks count in the x,y,z directions."
    echo "               The total MPI rank count is Rx*Ry*Rz"
    echo "    (Ex,Ey,Ez) are the element count per rank in the x,y,z directions."
    echo "               The total element count in 1 rank is Ex*Ey*Ez"
    echo "    (Pbegin,Pend,Pstep) are the iteration limits over the polynomial orders"
    echo "               in any given element.  Pend is one-passed-the-last entry."
    echo "    CGcount is the number of CG iteration to perform."
}

EchoInfo () {
    echo "N>The input arguments received are R=($Rx,$Ry,$Rz) E=($Ex,$Ey,$Ez) P=($Pbegin,$Pend,$Pstep) CG=$CGcount"
    echo "N>Total counts Rtotal=$Rtotal  Etotal=$Etotal  PiterTotal=$PiterTotal"
    echo "N>fullTemplateFilename= $fullTemplateFilename"
    echo "N>execname= $execname  logname_make= $logname_make  logname_exec= $logname_exec"
}

Make_new_sizedata_file () {
    cp -f $fullTemplateFilename "$szDataFile"

    python findreplace.py "$szDataFile" 'Rx  = TMPLT' "Rx   = $Rx"
    python findreplace.py "$szDataFile" 'Ry  = TMPLT' "Ry   = $Ry"
    python findreplace.py "$szDataFile" 'Rz  = TMPLT' "Rz   = $Rz"

    python findreplace.py "$szDataFile" 'Ex = TMPLT' "Ex   = $Ex"
    python findreplace.py "$szDataFile" 'Ey = TMPLT' "Ey   = $Ey"
    python findreplace.py "$szDataFile" 'Ez = TMPLT' "Ez   = $Ez"

    python findreplace.py "$szDataFile" 'Pbegin = TMPLT' "Pbegin = $Pbegin"
    python findreplace.py "$szDataFile" 'Pend   = TMPLT' "Pend   = $Pend"
    python findreplace.py "$szDataFile" 'Pstep  = TMPLT' "Pstep  = $Pstep"

    python findreplace.py "$szDataFile" 'CGcount = TMPLT' " CGcount = $CGcount"

    mv -f "$szDataFile" '../src'
}

Make_names () {
    label="R$Rx-$Ry-$Rz"
    label+="E$Ex-$Ey-$Ez"
    label+="P$Pbegin-$Pend-$Pstep"
    label+="CG$CGcount"

    execname='nekskeleton.x.'
    execname+="$label"

    logname_make='z_log_make.'
    logname_make+="$label"

    logname_exec='z_log_exec.'
    logname_exec+="$label"

    mpi_stats_filename='z_log_mpiStats.'
    mpi_stats_filename+="$label"
}

Build_the_executable () {
    cd '../src'

    execLocalName='nekskeleton.x'

    rm -f "$execLocalName"
    make > "$logname_make"
    if [ ! -s "$execLocalName" ]; then
        echo "ERROR: Building $execLocalName failed."
        erri=60
    else
        mv -f "$execLocalName" "../bin/$execname"
        mv -f "$logname_make" "../bin/$logname_make"
        erri=0
    fi

    cd "$path2here"
}

Run_the_code () {
    cd '../bin'
    local_exec='./'
    local_exec+="$execname"

    HOSTLIST="bar16,bar3,bar6,bar9,bar10,bar11,bar12,bar13,bar14,bar15,bar17,bar18,bar28,bar29,bar30,bar32"
#    HOSTLIST="bar16,bar3"

    #Do not forget to set I_MPI_STATS=all or I_MPI_STATS=10 or I_MPI_STATS=20

    export I_MPI_STATS_FILE="$mpi_stats_filename"

    commande="mpirun -n $Rtotal -hosts $HOSTLIST $local_exec"
    echo "N>$commande &> $logname_exec"

    $commande &> "$logname_exec"

    if [ "$(grep -i ERROR $logname_exec)" ]; then
        echo 'ERROR found in execution log.'
        erri=90
    fi

    cd "$path2here"
}

#----- Data acquisition
erri=0

argCount=$#

if [ ! $argCount = 10 ]; then
    echo "ERROR: input argCount=$argCount    10 input arguments are needed."
    Usage
fi

Rx=$1
Ry=$2
Rz=$3
Ex=$4
Ey=$5
Ez=$6
Pbegin=$7
Pend=$8
Pstep=$9
CGcount=${10}
Rtotal="$((Rx * Ry * Rz))"
Etotal="$((Ex * Ey * Ez))"

PiterTotal=0
P=$Pbegin
while [ $P -lt $Pend ];
do
    PiterTotal="$((PiterTotal + 1))"
    P="$((P + Pstep))"
done

execname=""
logname_make=""
logname_exec=""
mpi_stats_filename=""
Make_names

path2here="$(pwd)"

EchoInfo

#---- Start to work
Make_new_sizedata_file

Build_the_executable
if [ $erri != 0 ]; then
    exit "$erri"
fi

Run_the_code
if [ $erri != 0 ]; then
    echo "ERROR: $erri"
    exit "$erri"
fi

#----- Cleaning up
cd ../src
make clean &> /dev/null
cd "$path2here"
unset Run_the_code
unset Build_the_executable
unset Make_names
unset Make_new_sizedata_file
unset EchoInfo
unset Usage

exit 0
