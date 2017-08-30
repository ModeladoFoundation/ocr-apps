#!/bin/bash

echo " "
echo "#+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+##+#"
echo " "
echo "Host: " `hostname`
echo " "

CPU=19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34
#CPU=10,80,25,95,43,112,59,131
echo "CPUs: " $CPU
echo " "


echo " "
echo "Test cache Line Offset = 0"
LOG=cacheOffset_0.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=0 -cpu=$CPU >> $LOG


echo " "
echo "Test cache Line Offset = 64"
LOG=cacheOffset_64.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=64 -cpu=$CPU >> $LOG


echo " "
echo "Test cache Line Offset = 128"
LOG=cacheOffset_128.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=128 -cpu=$CPU >> $LOG


echo " "
echo "Test cache Line Offset = 256"
LOG=cacheOffset_256.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=256 -cpu=$CPU >> $LOG


echo " "
echo "Test cache Line Offset = 512"
LOG=cacheOffset_512.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=512 -cpu=$CPU >> $LOG



echo " "
echo "Test cache Line Offset = 96"
LOG=cacheOffset_96.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=96 -cpu=$CPU >> $LOG


echo " "
echo "Test cache Line Offset = 756"
LOG=cacheOffset_756.log
rm -rf $LOG
echo "Test #0"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
echo "Test #1"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
echo "Test #2"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
echo "Test #3"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
echo "Test #4"
rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
#echo "Test #5"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
#echo "Test #6"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
#echo "Test #7"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
#echo "Test #8"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG
#echo "Test #9"
#rapp xeonNumaSize -drSize=64M -cacheLoops=1000 -cacheOff=756 -cpu=$CPU >> $LOG

