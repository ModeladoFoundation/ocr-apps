os = serial_bfs.o
es = serial_bfs

cc = gcc
ccopts = -O1 -c -Wall
link = gcc
linkos = -o $(es) -lrt -g -lm

all : $(es)

$(es) : $(os)
	$(link) $(linkos) $(os)

serial_bfs.o : serial_bfs.c serial_bfs.h
	$(cc) $(ccopts) serial_bfs.c

clean : 
	rm -f *.o $(es) 2>/dev/null
