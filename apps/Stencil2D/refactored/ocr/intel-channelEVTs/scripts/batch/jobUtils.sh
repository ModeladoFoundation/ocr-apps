
# factor all the code

function submitBatchList() {
    local blist="$1"
    # Default naming if
    if [[ -z ${EXP_OUTDIR} ]]; then
        EXP_NAME=${EXP_NAME-"exp"}
        CURDATE=`date +%F_%Hh%M`
        EXP_OUTDIR=`mktemp -d job_output_${EXP_NAME}_${CURDATE}.XXX`
    fi
    cd ${EXP_OUTDIR}
    for batch in `echo "$blist"`; do
        sbatch ${batch}
    done | tee jobs_ids_tmp
    more jobs_ids_tmp | cut -d' ' -f4-4 > jobs_ids
    rm -f jobs_ids_tmp
}

function cancelAll() {
    XP_FOLDER="$1"
    # TODO check arg card and validity

    JOBS_FILE="${XP_FOLDER}/jobs_ids"

    if [[ ! -f ${JOBS_FILE} ]]; then
        echo "error no jobs_ids file"
    fi
    for job in `echo ${FILES}`; do
        scancel $job
    done
}

function isDone() {
    XP_FOLDER="$1"
    # TODO check arg card and validity

    JOBS_FILE="${XP_FOLDER}/jobs_ids"

    if [[ ! -f ${JOBS_FILE} ]]; then
        echo "error no jobs_ids file"
    fi

    # Assume username is the same used to submit jobs
    # Prints jobId along with status
    let runningJobs=0;
    let allJobs=0;
    FILES=`more ${JOBS_FILE} | tr '\n' ' '`
    for job in `echo ${FILES}`; do
        RES=`squeue -h -j ${job} -o "%t"`
        if [[ "$RES" != "" ]]; then
            let runningJobs=$runningJobs+1
        fi
        let allJobs=${allJobs}+1
    done

    if [[ ${runningJobs} -eq 0 ]]; then
        echo "All ${allJobs} jobs completed"
        exit 0
    else
        echo "${runningJobs}/${allJobs} jobs are still running"
        exit 1
    fi

}