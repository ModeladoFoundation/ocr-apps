
for((i=1024;i<524288; i=i*2))
do
make -f Makefile.x86 run CONFIG_NUM_THREADS=16 WORKLOAD_ARGS="$i 1024" >>LCS_OCR_10_14_2016.txt
done

wait



for((i=1;i<=15;i=i+2))
do
make -f Makefile.x86 run CONFIG_NUM_THREADS=$i WORKLOAD_ARGS="262144 1024" >>Scalability_10_14_2016.txt
done
wait
