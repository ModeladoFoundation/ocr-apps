#!/bin/bash

cd /global/panfs/users1/Xdelrod/src/apps/apps/examples/xeonNumaSize

echo " "
echo "#+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+#"
echo " "
echo "Host: " `hostname`
echo " "

# 8 CPUs, 4 on each of 2 cores, both cores in NUMA Node 2
CPU=50,51,118,119,186,187,254,255
echo "CPUs: " $CPU
echo " "

rm -f drSeq.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU" >> drSeq.log
echo -n "[drSeq.log - #1]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #2]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #3]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #4]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #5]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #6]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #7]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #8]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #9]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log
echo -n "[drSeq.log - #10]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq.log


rm -f mcSeq.log
echo " "
echo "++++++++++ Switched to ekf358"
echo "rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU" >> mcSeq.log
echo -n "[mcSeq.log - #1]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #2]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #3]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #4]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #5]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #6]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #7]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #8]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #9]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log
echo -n "[mcSeq.log - #10]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq.log

rm -f drRand.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU" >> drRand.log
echo -n "[drRand.log - #1]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #2]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #3]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #4]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #5]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #6]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #7]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #8]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #9]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log
echo -n "[drRand.log - #10]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand.log

rm -f mcRand.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU" >> mcRand.log
echo -n "[mcRand.log - #1]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #2]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #3]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #4]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #5]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #6]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #7]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #8]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #9]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log
echo -n "[mcRand.log - #10]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand.log




# 2 CPUs in each NUMA node (0-3), each on a different core, 
CPU=10,80,25,95,43,112,59,131


rm -f drSeq4.log
echo " "
echo "##########"
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU" >> drSeq4.log
echo -n "[drSeq4.log - #1]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #2]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #3]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #4]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #5]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #6]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #7]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #8]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #9]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log
echo -n "[drSeq4.log - #10]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq4.log

rm -f mcSeq4.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU" >> mcSeq4.log
echo -n "[mcSeq4.log - #1]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #2]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #3]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #4]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #5]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #6]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #7]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #8]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #9]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log
echo -n "[mcSeq4.log - #10]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq4.log

rm -f drRand4.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU" >> drRand4.log
echo -n "[drRand4.log - #1]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #2]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #3]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #4]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #5]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #6]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #7]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #8]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #9]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log
echo -n "[drRand4.log - #10]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand4.log

rm -f mcRand4.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU" >> mcRand4.log
echo -n "[mcRand4.log - #1]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #2]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #3]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #4]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #5]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #6]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #7]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #8]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #9]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log
echo -n "[mcRand4.log - #10]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand4.log





# 16 CPUs all on NUMA Node 2
CPU=19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34


rm -f drSeq5.log
echo " "
echo "##########"
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU" >> drSeq5.log
echo -n "[drSeq5.log - #1]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #2]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #3]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #4]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #5]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #6]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #7]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #8]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #9]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log
echo -n "[drSeq5.log - #10]   -  "; date
rapp xeonNumaSize -drSize=64M -drRLoops=1000 -cpu=$CPU >> drSeq5.log

rm -f mcSeq5.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU" >> mcSeq5.log
echo -n "[mcSeq5.log - #1]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #2]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #3]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #4]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #5]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #6]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #7]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #8]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #9]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log
echo -n "[mcSeq5.log - #10]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRLoops=1000 -cpu=$CPU >> mcSeq5.log

rm -f drRand5.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU" >> drRand5.log
echo -n "[drRand5.log - #1]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #2]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #3]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #4]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #5]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #6]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #7]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #8]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #9]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log
echo -n "[drRand5.log - #10]   -  "; date
rapp xeonNumaSize -drSize=64M -drRRLoops=1000 -cpu=$CPU >> drRand5.log

rm -f mcRand5.log
echo " "
echo "++++++++++"
echo "rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU" >> mcRand5.log
echo -n "[mcRand5.log - #1]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #2]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #3]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #4]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #5]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #6]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #7]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #8]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #9]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log
echo -n "[mcRand5.log - #10]   -  "; date
rapp xeonNumaSize -mcSize=64M -mcRRLoops=1000 -cpu=$CPU >> mcRand5.log



