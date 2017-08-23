CONFIGS="-regular -nomg-nohalo -noreduction -noreduction-nomg -noreduction-nomg-nohalo"
BASECPGN=${BASECPGN-"xp-benchmark-hpcgEagerRedlib-wrank-itone-cent"}

for c in `echo $CONFIGS`; do
    export CPGN="$BASECPGN${c}"
    echo "${CPGN}"
    ./scripts/summaryRankTimer.sh
done