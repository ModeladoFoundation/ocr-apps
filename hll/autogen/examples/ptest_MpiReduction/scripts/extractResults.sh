#!/bin/bash

CPGN=$1

#TODO this should be tokenized
if [[ -z "${CPGN}" ]]; then
    echo "error: usage ./extractResults.sh campaign_folder_path"
    exit 1
fi

TMP=tmpresfile
for resFile in `find ${CPGN}/jobdir/job_output* -name "allreduce_N*" | sed -e "s|N| |g" | sort -t ' ' -k 2,2 -n | sed -e "s| |N|g"`; do
    echo "$resFile"
    more $resFile | grep Throughput | cut -d' ' -f5-5 > ${TMP}
    python ./scripts/y_calc_stats.py ${TMP}
    rm -Rf ${TMP}
done
