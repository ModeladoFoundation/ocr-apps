

for((i=1024;i< 524288; i=i*2))
do
./lcs $i 16 1024 >> LCS_CILK_10_14_2016.txt
done

wait

for((i=1;i<=19;i=i+2))
do
./lcs 262144 $i 1024 >> LCS_CILK_Scalability_10_14_2016.txt
done

wait
./lcs 262144 16 1024 >> LCS_CILK_Scalability_10_14_2016.txt
wait
