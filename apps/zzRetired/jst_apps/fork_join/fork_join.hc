#include <hc.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>

void computation_phase(int comp_weight, int *in_out);

inline unsigned long time_to_us(struct timespec ts) {
	return (ts.tv_nsec + (1000000000UL * ts.tv_sec))/1000UL;
}

struct cmdline {
	int parallel_weight;
	int parallel_tasks;
	int serial_weight;
	int nphases;
	int parfirst;
};

static struct cmdline default_cmdline = {
	100, 100, 100, 100, 0
};

static struct option long_options[] = {
	{"parweight", 	required_argument, 	NULL, 	'p' },
	{"partasks",  	required_argument, 	NULL, 	't' },
	{"serial",    	required_argument, 	NULL, 	's' },
	{"nphases",   	required_argument, 	NULL, 	'n' },
	{"parfirst",	no_argument,		NULL,	'f' },
	{0, 	    	0,					0,		0}
};

static char *descriptions[] = {
	"\t\tWeight of the parallel calculation. Relative amount of work each task\n\t\tin the parallel section performs",
	"\t\tNumber of parallel tasks",
	"\t\tWeight of the serial calculation.",
	"\t\tNumber of phases (serial/parallel pairs) to execute.",
	"\t\tIndicate if the parallel section should execute first in a phase. If this\n\t\tflag is not set, then the serial section will run first."
};

void print_usage(char * exename) {
	int optindex=0;
	printf("NAME\n\t%s - an iterative serial-parallel synthetic workload\n\n", exename);
	printf("SYNOPSIS\n\t%s [options]\n\n", exename);
	printf("DESCRIPTION\n");
	printf("\tThis workload doesn't do real work, but it does it's fake work in a possibly\n");
	printf("\tinteresting fashion. The work is broken into a set of parallel and serial\n");
	printf("\tphases, and the phases are iteratively executed. On can control the relative\n");
	printf("\tweights of the parallel and serial portions, the number oftasks launched in\n");
	printf("\tthe parallel portions, and how many phases to execute on the commandline.\n\n");
	printf("OPTIONS:\n");
	while(long_options[optindex].name != 0) {
		printf("\t-%c, --%s [num]\n%s\n\n",
			(char)long_options[optindex].val,
			long_options[optindex].name,
			descriptions[optindex]);
		optindex++;
	}
}

struct cmdline parse_cmdline(int argc, char *argv[])
{
	int opt, index;
	while ((opt = getopt_long_only(argc, argv, "p:t:s:n:hf",
					long_options, &index)) != -1) {
		switch(opt) {
			case 'p':
				default_cmdline.parallel_weight= atoi(optarg);
				break;
			case 't':
				default_cmdline.parallel_tasks = atoi(optarg);
				break;
			case 's':
				default_cmdline.serial_weight  = atoi(optarg);
				break;
			case 'n':
				default_cmdline.nphases        = atoi(optarg);
				break;
			case 'f':
				default_cmdline.parfirst = 1;
				break;
			case 'h':
			default: // Fallthough on purpose
				print_usage(argv[0]);
				exit(42);
				break;
		}
	}
	return default_cmdline;
}

void print_options(struct cmdline mycmdline)
{
	printf("Running with options:\n");
	printf("\t%-12d Parallel Task Runtime\n", mycmdline.parallel_weight);
	printf("\t%-12d Number of parallel tasks\n", mycmdline.parallel_tasks);
	printf("\t%-12d Serial Task Runtime\n", mycmdline.serial_weight);
	printf("\t%-12d Number of parallel serial phases\n", mycmdline.nphases);
	printf("\t%-12s In a phase, does the parallel section run first?\n",
			(mycmdline.parfirst == 1) ? "true" : "false");
}

int main(int argc, char *argv[])
{
	int i,j;
	struct cmdline mycmdline = parse_cmdline(argc, argv);
	int *result_arr = calloc(mycmdline.parallel_tasks+1, sizeof(int));
	struct timespec startt, endt;

	print_options(mycmdline);

	clock_gettime(CLOCK_MONOTONIC_RAW, &startt);
	for (i=0;i<mycmdline.nphases;i++) {
		result_arr[mycmdline.parallel_tasks] += i;
		if (!mycmdline.parfirst) {
			computation_phase(mycmdline.serial_weight, &result_arr[mycmdline.parallel_tasks]);
		}
		finish {
			for (j=0;j<mycmdline.parallel_tasks;j++) {
				async IN(mycmdline, j, i, result_arr) {
					result_arr[j] += j + i;
					computation_phase(mycmdline.parallel_weight, &result_arr[j]);
				}
			}
		} // Wait here for all the parallel tasks to finish
		if (mycmdline.parfirst) {
			computation_phase(mycmdline.serial_weight, &result_arr[mycmdline.parallel_tasks]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &endt);

	printf("Result: %ldus per phase\n",
			(time_to_us(endt) - time_to_us(startt))/((long)mycmdline.nphases));

	free(result_arr);
	return 0;
}


#define INNER_LOOP 100
void computation_phase(int comp_weight, int *in_out)
{
	int i,j;
	for (i=0;i<comp_weight;i++) {
		for (j=0;j<INNER_LOOP;j++) {
			*in_out = *in_out ^ (i ^ j);
		}
	}
}
