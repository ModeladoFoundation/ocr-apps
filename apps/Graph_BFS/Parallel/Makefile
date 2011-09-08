os = parallel_bfs.o
es = parallel_bfs

cc = gcc
ccopts = -O1 -c -Wall
link = gcc
linkos = -o $(es) -lrt -g -lm -lpthread

all : $(es)

$(es) : $(os)
	$(link) $(linkos) $(os)

parallel_bfs.o : parallel_bfs.c parallel_bfs.h
	$(cc) $(ccopts) parallel_bfs.c

clean : 
	rm -f *.o $(es) 2>/dev/null
