#!/bin/sh

function submit_jobs() {
    target=$1
    dir=$2
    cd ./$dir
    slurm_file=`ls $target.16-*.slurm`
    echo "Submitting $slurm_file"
    sbatch $slurm_file
    slurm_file=`ls $target.32-*.slurm`
    echo "Submitting $slurm_file"
    sbatch $slurm_file
    slurm_file=`ls $target.64-*.slurm`
    echo "Submitting $slurm_file"
    sbatch $slurm_file
    slurm_file=`ls $target.128-*.slurm`
    echo "Submitting $slurm_file"
    sbatch $slurm_file
    slurm_file=`ls $target.256-*.slurm`
    echo "Submitting $slurm_file"
    sbatch $slurm_file
    slurm_file=`ls $target.512-*.slurm`
    echo "Submitting $slurm_file"
    sbatch $slurm_file
}

if [[ $# -ne 2 ]]; then
    echo "Expecting target and directory name"
    exit 1
fi

submit_jobs $1 $2
