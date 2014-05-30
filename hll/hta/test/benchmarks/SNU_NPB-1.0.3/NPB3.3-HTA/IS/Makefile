SHELL=/bin/sh
BENCHMARK=is
BENCHMARKU=IS

include ../config/make.def

include ../sys/make.common

OBJS = is.o \
       ${COMMON}/c_print_results.o \
       ${COMMON}/c_timers.o \
       ${COMMON}/c_wtime.o


${PROGRAM}: config ${OBJS}
#	${CLINK} ${CLINKFLAGS} $(SWARM_FLAGS) $(SWARM_INC) -I${PIL_INC} -I${HTA_INC} -o ${PROGRAM}.swarm $(PIL_SRC) $(OP_SWARM) ${OBJS} ${C_LIB} -L$(HTA_LIB) -lhta $(SWARM_LINK)
	${CLINK} ${CLINKFLAGS} -DOMP -DPIL2C -DPILHTA -I${PIL_INC} -I${PIL_H} -I${HTA_INC} -o ${PROGRAM}.omp $(PIL_SRC) $(OP_OMP) ${OBJS} ${C_LIB} -L$(HTA_LIB) -lhta -lm
#	${CLINK} ${CLINKFLAGS} -DPIL2C -DPILHTA -I${PIL_INC} -I${HTA_INC} -o ${PROGRAM}.seq $(PIL_SRC) $(OP_SEQC) ${OBJS} ${C_LIB} -L$(HTA_LIB) -lhta
	$(CLINK) ${CLINKFLAGS} $(OCR_FLAGS) -DPIL2OCR -I${PIL_INC} -I${HTA_INC} -I${PIL_H} $(OP_OCR) $(OBJS) ${C_LIB} $(PIL_SRC) -o ${PROGRAM}.ocr $(OCR_LDFLAGS) -L${HTA_LIB} -lhta

.c.o:
	${CCOMPILE} $<

is.o:             is.c  npbparams.h


clean:
	- rm -f *.o *~ mputil*
	- rm -f npbparams.h core
	- if [ -d rii_files ]; then rm -r rii_files; fi
