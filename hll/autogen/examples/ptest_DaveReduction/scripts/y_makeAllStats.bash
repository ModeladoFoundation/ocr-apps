#!/usr/bin/env bash

if [ $# -ne 1 ]
then
    echo 'USAGE: <exec> <input data file>'
    echo '=This script takes the name of the input file and parse its content'
    echo ' in order to calculates some statistics.'
    echo '=The content of the file must be the log output of ptest_DaveReduction$.'
fi

OUTF='z_zz_temp'
FNAME=$1

#Processing lines
#   INFO: TIME: ffjFinalEdt_fcn_time_mark= 1487053690878790
#   INFO: TIME: ffjMainEdt_fcn_time_mark=  1487053690877934
a="$(grep INFO $FNAME|grep mark|tr -s ' ' '\t'|cut -f4)"
b=($a)
c="$(( ${b[1]} - ${b[0]} ))"
#dbg> echo $c
TOTAL_RUN_TIME="$(echo "scale=6;$c/1000000" | bc)"
echo "FNAME= $FNAME"
echo "TotalRunTime= $TOTAL_RUN_TIME in seconds"
echo "All other measurements in micro-seconds."

#Processing for number of iterations
#   INFO: FFJ_Ledger: Number of iterationB requested= 100
LABEL='iterationB'
ITER_COUNT="$(grep iterationB z_log|tr '=' '\n'|grep -v iteration)"
echo "$LABEL= $ITER_COUNT"

#Processing lines for reduxA
#   INFO: TIME: DR rank=1/8 reduxA,reduxB,cumulReduxB=1402,408,38570
LABEL='reduxA'
A="$(grep INFO $FNAME|grep $LABEL|tr -s ' ' '\t'|cut -f5|tr '=' '\n'|grep -v $LABEL|cut -d',' -f1)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%16s  %s\n' "$LABEL" "$STATS"

#Processing lines for reduxB
#   INFO: TIME: DR rank=1/8 reduxA,reduxB,cumulReduxB=1402,408,38570
LABEL='reduxB'
A="$(grep INFO $FNAME|grep $LABEL|tr -s ' ' '\t'|cut -f5|tr '=' '\n'|grep -v $LABEL|cut -d',' -f2)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%16s  %s\n' "$LABEL" "$STATS"

#Processing lines for cumulReduxB
#   INFO: TIME: DR rank=1/8 reduxA,reduxB,cumulReduxB=1402,408,38570
LABEL='cumulReduxB'
A="$(grep INFO $FNAME|grep $LABEL|tr -s ' ' '\t'|cut -f5|tr '=' '\n'|grep -v $LABEL|cut -d',' -f3)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%16s  %s\n' "$LABEL" "$STATS"

#Processing lines for scatter
#   INFO: TIME: rank=0/8 scatter,FORtransit= 94,648
LABEL='scatter'
A="$(grep INFO $FNAME|grep $LABEL|tr -s ' ' '\t'|cut -f5|tr '=' '\n'|grep -v $LABEL|cut -d',' -f1)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%16s  %s\n' "$LABEL" "$STATS"

#Processing lines for FORtransit
#   INFO: TIME: rank=0/8 scatter,FORtransit= 94,648
LABEL='FORtransit'
A="$(grep INFO $FNAME|grep $LABEL|tr -s ' ' '\t'|cut -f5|tr '=' '\n'|grep -v $LABEL|cut -d',' -f2)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%16s  %s\n' "$LABEL" "$STATS"

