#OBJS=db_alloc_pthrhack.o simple_allocator.o ../common/hash_table.o ../common/core_id.o
CFLAGS=-DDEBUG=2 -g -Wall -Irmd_afl/common -Irmd_afl/memory_allocation

os = parallel_bfs.o
es = parallel_bfs

cc = gcc
ccopts = -O1 -c -Wall $(CFLAGS)
link = gcc
linkos = -lrt -g -lm -lpthread rmd_afl/memory_allocation/db_alloc.a -o $(es)

all : $(es)

$(es) : $(os)
	$(link) $(os) $(linkos)

parallel_bfs.o : parallel_bfs.c parallel_bfs.h rmd_super_header.h
	$(cc) $(ccopts) parallel_bfs.c

#db_alloc.a : $(OBJS)
#	$(AR) rcs $A $(OBJS)

clean : 
	rm -f *.o $(es) 2>/dev/null
