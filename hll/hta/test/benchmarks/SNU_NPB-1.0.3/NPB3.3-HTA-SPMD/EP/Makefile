SHELL=/bin/sh
BENCHMARK=ep
BENCHMARKU=EP

include ../config/make.def

include ../sys/make.common

OBJS = ep.o \
			 ${COMMON}/print_results.o \
			 ${COMMON}/${RAND}.o \
       ${COMMON}/c_timers.o \
			 ${COMMON}/wtime.o


${PROGRAM}: config ${OBJS}
	#${CLINK} ${CLINKFLAGS} $(SWARM_FLAGS) $(SWARM_INC) -I${PIL_INC} -I${HTA_INC} -o ${PROGRAM}.swarm $(PIL_SRC) $(OP_SWARM) ${OBJS} ${C_LIB} -L$(HTA_LIB) ${LHTA} $(SWARM_LINK)
	${CLINK} ${CLINKFLAGS} -DOMP -DPIL2C -DPILHTA -I${PIL_INC} -I${PIL_H} -I${HTA_INC} -o ${PROGRAM}.omp $(PIL_SRC) $(OP_OMP) ${OBJS} ${C_LIB} -L$(HTA_LIB) ${LHTA} -lpthread
#	${CLINK} ${CLINKFLAGS} -DPIL2C -DPILHTA -I${PIL_INC} -I${HTA_INC} -o ${PROGRAM}.seq $(PIL_SRC) $(OP_SEQC) ${OBJS} ${C_LIB} -L$(HTA_LIB) -lhta
	$(CLINK) ${CLINKFLAGS} $(OCR_FLAGS) -DPIL2OCR -I${PIL_INC} -I${HTA_INC} -I${PIL_H} $(OP_OCR) $(OBJS) ${C_LIB} $(PIL_SRC) -o ${PROGRAM}.ocr $(OCR_LDFLAGS) -L${HTA_LIB} ${LHTA}
#	#$(MPICC) ${CLINKFLAGS} -I${HTA_INC} $(OP_MPI) ${OBJS} ${C_LIB} -o ${PROGRAM}.mpi -L${HTA_LIB} -lhta_mpi

.c.o:
	${CCOMPILE} $<

ep.o: ep.c npbparams.h

clean:
	- rm -f *.o *~
	- rm -f npbparams.h core
	- if [ -d rii_files ]; then rm -r rii_files; fi
