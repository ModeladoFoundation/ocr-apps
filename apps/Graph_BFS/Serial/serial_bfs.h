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
#include <sys/time.h>
#include <sys/resource.h>

typedef struct node node;
typedef struct edgeList edgeList;

//elements in an adjacency list. Currently its an uninteresting graph
//But I am making a structure (as opposed to a primitive) should it
//be desirable to make it more interesting.
struct node {
	int id;
	int marked;
	//changed to a linked list for no real reason
	//int numNeighbors;
	struct edgeList *neighbors;
};

struct edgeList {
	struct node *neighbor;
	struct edgeList* next;
};

int numNodes;
node **hashList = NULL;

//run BFS from graph[start] until numNodes have been explored
int* BFS(node **graph, int numNodes, int start);
node** BuildGraph(char *filename);
node *parseEdgeList(char *str);
void printBFS(FILE* fd, int* traversal, node** graph, int numNodes);
void printGraph(FILE* fd, node** graph, int numNodes);
void printNode(FILE* fd, node* pnode, int mode);
node *hashNode(int key);
