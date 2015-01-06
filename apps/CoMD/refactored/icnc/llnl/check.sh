#!/bin/bash
cat output/13atoms_fff_v1_dump10-3-3-3-1000-1.out | sort > 111; cat ../output/13atoms_fff_v1_dump10.cnc | sort > 222; echo Diff 13fff; diff -q 111 222; rm 111 222
cat output/55atoms_fff_v1_dump10-3-3-3-1000-1.out | sort > 111; cat ../output/55atoms_fff_v1_dump10.cnc | sort > 222; echo Diff 55fff; diff -q 111 222; rm 111 222
cat output/64atoms_fff_v1_dump1-3-3-3-1000-1.out | sort > 111; cat ../output/64atoms_fff_v1_dump1.cnc | sort > 222; echo Diff 64fff; diff -q 111 222; rm 111 222
cat output/7atoms_fff_v3_dump1-3-3-3-100-1.out | sort > 111; cat ../output/7atoms_fff_v3_dump1.cnc | sort > 222; echo Diff 7fff; diff -q 111 222; rm 111 222
cat output/351atoms_fff_v1_dump100-3-3-3-10000-100.out | sort > 111; cat ../output/351atoms_fff_v1_dump100.cnc | sort > 222; echo Diff 351fff; diff -q 111 222; rm 111 222
cat output/440atoms_fff_v1_dump100-3-3-3-11200-100.out | sort > 111; cat ../output/440atoms_fff_v1_dump100.cnc | sort > 222; echo Diff 440fff; diff -q 111 222; rm 111 222
cat output/864atoms_fff_v1_dump10-3-3-3-1000-10.out | sort > 111; cat ../output/864atoms_fff_v1_dump10.cnc | sort > 222; echo Diff 864fff; diff -q 111 222; rm 111 222
cat output/864atoms_fff_v300_dump10_realunits-3-3-3-1000-10.out | sort > 111; cat ../output/864atoms_fff_v300_dump10_realunits.cnc | sort > 222; echo Diff 864fffreal; diff -q 111 222; rm 111 222
