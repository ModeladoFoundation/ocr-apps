CFLAGS  = -g -O3 -fopenmp
LDFLAGS = -fopenmp -lm

APP     = cg
OBJECTS = cg_omp.o main.o util.o makea.o

.PHONY: all

all: $(APP)

$(OBJECTS): $(APP).h

$(APP): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

.PHONY: clean

clean:
	$(RM) $(APP) $(OBJECTS)
