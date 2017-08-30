export TESTDIR=$PWD

cd ../..

echo $PWD

make

cd $TESTDIR

echo $PWD

make clean all

./TaskTest

