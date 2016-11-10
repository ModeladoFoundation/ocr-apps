#!/usr/bin/env bash

if [ $# -ne 1 ]
then
    echo 'USAGE: <exec> <input data file>'
    echo '=This script takes the name of the input file and parse its content'
    echo ' in order to calculates some statistics.'
    echo '=The content of the file must be the log output of OCRed Nekbone.'
fi

OUTF='z_zz_temp'
FNAME=$1

a="$(grep NKTIME z_log|grep -E "(FinalEDT|init_NEKOstatics)"|tr -s '=' '\n'|grep -v NKTIME)"
b=($a)
c="$(( ${b[1]} - ${b[0]} ))"
#echo $c
TOTAL_RUN_TIME="$(echo "scale=6;$c/1000000" | bc)"
echo "TotalRunTime= $TOTAL_RUN_TIME in seconds"
echo "All other measurements in micro-seconds."

LABEL='RankSetup'
A="$(grep NKTIME $FNAME | grep $LABEL | tr ' ' '\n'| grep $LABEL | tr '=' '\n'|grep -v $LABEL)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='TotalRank'
A="$(grep NKTIME $FNAME | grep $LABEL | tr ' ' '\n'| grep $LABEL | tr '=' '\n'|grep -v $LABEL)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='SPDM_fork'
A="$(grep NKTIME $FNAME | grep $LABEL | tr ' ' '\n'| grep $LABEL | tr '=' '\n'|grep -v $LABEL)"
#dbg> echo $A
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_tailRecurTransitBEGIN'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_solveMi'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_beta_start'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_beta_stop'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_axi_start'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_axi_stop'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_alpha_start'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_alpha_stop'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_rtr_start'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_rtr_stop'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"


LABEL='cumu_nekCG_rtr_transit'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_alpha_transit'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_beta_transit'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"

LABEL='cumu_nekCG_axi_transit'
A="$(grep $LABEL z_log | tr ' ' '\n' | grep $LABEL | tr '=' '\n' | grep -v $LABEL)"
printf "%s\n" "${A[@]}" > $OUTF
STATS="$(python y_calc_stats.py $OUTF)"
printf '%32s  %s\n' "$LABEL" "$STATS"
