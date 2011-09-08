#include "serial_bfs.h"

int main(int argc, char *argv[])
{
	char *file = (char *) "/home/gibbons/APPS/BFS/Input/default.txt";
	if(argc > 1) file = argv[1];
	node** graph = BuildGraph(file);

	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);
	int* traversal = BFS(graph, numNodes, 0);
	gettimeofday(&endTime, NULL);;

	int conversion = 1000000;

	double tS = startTime.tv_sec*conversion + (startTime.tv_usec);
	double tE = endTime.tv_sec*conversion + (endTime.tv_usec);

	printf("Total processor usage: %f seconds\n", (tE-tS)/conversion);

	//printBFS(stdout, traversal, graph, numNodes);
	return 0;
}

int* BFS(node **graph, int numNodes, int start)
{
	int pop_index = 0;
	int push_index = 1;
	int *order = (int *) malloc(numNodes*sizeof(int));
	order[pop_index] = start;
	graph[start]->marked = 1;
	for(; pop_index < numNodes; pop_index++)
	{
		//graph is not connected. start randomly...
		if(push_index == pop_index)
		{
			int new_index = 0;
			while(graph[new_index]->marked == 1)
				new_index++;
			graph[new_index]->marked = 1;
			order[push_index] = graph[new_index]->id;
			push_index++;
		}

		node* curr = graph[order[pop_index]];
		edgeList *list = curr->neighbors;
		while(list != NULL)
		{
			node* neighbor = list->neighbor;
			if(neighbor->marked == 0) 
			{
				neighbor->marked = 1;
				order[push_index] = neighbor->id;
				push_index++;
			}

			list = list->next;
		}

	}

	return order;
}

node** BuildGraph(char *filename)
{
	FILE *fd;
	fd = fopen(filename, "r");
	if(fd == NULL) {printf("Input File: %s could not be opened for reading\n", filename); exit(1);}

	char num_str[80];
	numNodes = atoi(fgets(num_str, 80, fd));
	node** graph = (node **) malloc(sizeof(node)*numNodes);

	//how big this buffer needs to be is of some mild interest... and is currently over allocated
	int BUFF_SIZE = numNodes*numNodes;
	char *str = (char *) malloc(sizeof(char)*BUFF_SIZE);
	int i = 0;
	for(; i < numNodes; i++)
	{
		fgets(str, BUFF_SIZE, fd);
		graph[i] = parseEdgeList(str);
	}

	return graph;
}

node *parseEdgeList(char *str)
{
	char *ptr = str;
	while(*ptr != ' ')
	{
		ptr++;
	}
	*ptr = '\0';
	node *new_node = hashNode(atoi(str));
	str = ++ptr;
	int flag = 0;
	while(flag == 0)
	{
		while(*ptr != ' ' && *ptr != '\n' && *ptr != EOF) ptr++;
		if(*ptr == '\n' || *ptr == EOF) flag = 1;
		*ptr = '\0';
		int edge = atoi(str);

		node *neighbor = hashNode(edge);
		edgeList* neighbors = (edgeList *) malloc(sizeof(edgeList));
		neighbors->neighbor = neighbor;
		neighbors->next = new_node->neighbors;
		new_node->neighbors = neighbors;
	
		str = ++ptr;
	}

	return new_node;
}

//Because the keys are enumerated, hashing is easy.
//The reason I set up this grandiose system
//is to offer the flexibility to change this
//but it will never be changed...
node *hashNode(int key)
{
	if(hashList == NULL)
	{
		hashList = (node **) malloc(numNodes*sizeof(node *));
		int i = 0; for(; i< numNodes; i++) hashList[i] = NULL;
	}

	if(hashList[key] == NULL)
	{
		//methodize
		hashList[key] = (node *) malloc(sizeof(node));
		hashList[key]->id = key;
		hashList[key]->neighbors = NULL;
		hashList[key]->marked = 0;
	}

	return hashList[key];
}

void printGraph(FILE* fd, node **graph, int numNodes)
{
	int i = 0;
	for(; i < numNodes; i++)
		printNode(fd, graph[i], -1);
}

void printBFS(FILE* fd, int* traversal, node** graph, int numNodes)
{
	int i = 0;
	for(; i < numNodes; i++)
		printNode(fd, graph[traversal[i]], -2);
	printf("\n");
}

void printNode(FILE* fd, node *pnode, int mode)
{
	fprintf(fd, "%d ", pnode->id);
	if(mode == -1)
	{
		edgeList* list = pnode->neighbors;
		while(list != NULL)
		{
			printNode(fd, list->neighbor, -2);
			list = list->next;
		}	
	}
	
	if(mode != -2)
		fprintf(fd, "\n");
}
