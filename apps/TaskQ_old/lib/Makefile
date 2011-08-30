
taskQ.o: 
	icc -O3 -DCORES_PER_SOCKET=6 -DTASKQ_DIST_FIXED -c taskQDistCommon.c
	mv taskQDistCommon.o taskQ.o

clean:
		rm *.o
