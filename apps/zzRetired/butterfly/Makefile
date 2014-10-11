os = serial.o
es = serial

cc = gcc
ccopts = -O1 -c -Wall
link = gcc
linkos = -lrt -g -lm -o $(es)

all : $(es)

$(es) : $(os)
	$(link) $(os) $(linkos)

serial.o : serial.c serial.h
	$(cc) $(ccopts) serial.c

clean :
	rm -f *.o $(es) 2>/dev/null
