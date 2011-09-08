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

#define INFINITY 0x7FFFFFFF

typedef struct node node;
typedef struct edgeList edgeList;

//elements in an adjacency list. Currently its an uninteresting graph
//But I am making a structure (as opposed to a primitive) should it
//be desirable to make it more interesting.
struct node {
	int id;
	int height;
	//changed to a linked list for no real reason
	//int numNeighbors;
	//changing neighbors list to be a data minimal packet
	//in this case an integer
	struct edgeList *neighbors;
	struct node *next;

	//apparently Graph 500 doesn't generate an order
	//it just build an uptree
	//what chumps.
	struct node *upTree;
};

typedef struct new_node {
	int id;
	int height;
	struct new_node *next;
	node *upTree;
} new_node;


struct edgeList {
	struct node *neighbor;
	struct edgeList* next;
};

/*
typedef struct Communication {
	node *ptr;
	struct Communication* next;
} Communication;
*/
/*
typedef struct LinkedList {
	Communication *head;
	Communication *tail;
} LinkedList;
*/

typedef struct Thread_Package {
	int thread_id;
	node *start_node;
} Thread_Package;

int numNodes;
int numThreads;
int exploreDepth;
int global_flag = 1;
node **globalList = NULL;

pthread_barrier_t barr;

new_node ***Communication;

int master_count = 0;
int *master_order = NULL;
pthread_mutex_t master_lock;

//run BFS from graph[start] until numNodes have been explored
int* BFS(node **graph, int numNodes, int start, int numThreads);
void* BFS_worker(void *tp);
node** BuildGraph(char *filename);
node *parseEdgeList(char *str);
void printBFS(FILE* fd, int* traversal, node** graph, int numNodes);
void printGraph(FILE* fd, node** graph, int numNodes);
void printNode(FILE* fd, node* pnode, int mode);
node *hashNode(int key, node **);
void init_globals(int numThreads);

int reduction(node *curr);
node *pop(node **local_head_ref, int exploreDepth, node ***hashHeight_ref, node ***hashHeightTail_reg);
void explore(node *curr, int height, int thread_id);
void push(node **local_head_ref, int thread_id, node **hashNodes, node ***hashHeight_ref, node ***hashHeightTail_ref);
int mapping(int id);
void add(node **local_head_ref, new_node *temp, node **hashNodes, node ***hashHeight_ref, node ***hashHeightTail_ref);
int threadHash(int x);

