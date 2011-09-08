/*
 *	A serial implementation of Breadth First Search
 *	Created to test SESC for the UHPC project
 *
 *	This implementation takes up the motto 
 *	"Everything is a pointer! Pointers everywhere!"
 *	"p.s. everything is a hash table"
 *
 *	@author	: Austin Gibbons
 *	@date	: August 17, 2011
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define INFINITY 0x7FFFFFFF
#define bitsPerByte 8

typedef struct node node;
typedef struct edgeList edgeList;

typedef struct new_node {
	int id;
	struct new_node *next;
} new_node;

typedef struct Thread_Package {
	int thread_id;
	int start_id;
} Thread_Package;

int numNodes;
int numThreads;
int global_flag = 1;
int **glbl_Adjacency = NULL;
int *glbl_DepthParent = NULL;

//Checking for exploration
char *glbl_VIS;

pthread_barrier_t barr;

new_node ***Communication;
int **Communication_count;

int master_count = 0;
int *master_order = NULL;
pthread_mutex_t master_lock;

//run BFS from graph[start] until numNodes have been explored
int* BFS(int numNodes, int start, int numThreads);
void* BFS_worker(void *tp);
void BuildGraph(char *filename);
void parseEdgeList(char *str, int i, int BUFF_SIZE);
void init_globals(int numThreads);
void printDP();

int reduction(int current_size);
void explore(int id, int height, int thread_id);
int push(int **current_ref, int thread_id);
int mapping(int id);
int threadHash(int x);

