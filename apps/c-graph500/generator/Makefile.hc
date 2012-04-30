CC = gcc
CFLAGS = -std=c99 -Wall -O2 -I${HC_HOME}/hc
LDFLAGS = -lpthread -lxml2 -lhc

GENERATOR_SOURCES = graph_generator.c make_graph.c splittable_mrg.c utils.c
GENERATOR_HEADERS = graph_generator.h make_graph.h mod_arith_32bit.h mod_arith_64bit.h mod_arith.h mod_arith_xmt.h splittable_mrg.h utils.h user_settings.h mrg_transitions.c

all: generator_test_hc

generator_test_hc: generator_test_hc.c $(GENERATOR_SOURCES) $(GENERATOR_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o generator_test_hc generator_test_hc.c $(GENERATOR_SOURCES) -lm

clean:
	-rm -f generator_test_hc
