#include "parallel_bfs.h"

int main(int argc, char *argv[])
{
	exploreDepth = 1; //so magical
	numThreads = 2;
	char *file = (char *) "/home/gibbons/APPS/BFS/Input/default.txt";
	if(argc > 1) file = argv[1];
	if(argc > 2) numThreads = atoi(argv[2]);
	if(argc > 3) exploreDepth = atoi(argv[3]);	
	node** graph = BuildGraph(file);

	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);
	/*int* traversal =*/ BFS(graph, numNodes, 0, numThreads);
	gettimeofday(&endTime, NULL);

	int conversion = 1000000;
	double tS = startTime.tv_sec*conversion + (startTime.tv_usec);
	double tE = endTime.tv_sec*conversion + (endTime.tv_usec);
	printf("Total processor usage: %f seconds\n", (tE-tS)/conversion);

	//printBFS(stdout, traversal, graph, numNodes);
	return 0;
}

int* BFS(node **graph, int numNodes, int start, int numThreads)
{
	init_globals(numThreads);
	pthread_t *threads = (pthread_t *) malloc(numThreads*sizeof(pthread_t));
	Thread_Package *thread_package = (Thread_Package *) malloc(numThreads*sizeof(Thread_Package));

	int i = 0;
	//methodize
	int start_mapping = mapping(graph[start]->id);
	graph[start]->height = 0;
	//end
	
	for(; i < numThreads; i++)
	{
		thread_package[i].thread_id = i;
		thread_package[i].start_node = NULL;
		if(i == start_mapping)
			thread_package[i].start_node = graph[start];
		pthread_create(&threads[i], NULL, &BFS_worker, (void *) (&thread_package[i]));
	}

	//make main thread do work

	for(i = 0; i < numThreads; i++)
		pthread_join(threads[i], NULL);

	return NULL;
}

void *BFS_worker(void *tp)
{
	Thread_Package *x = (Thread_Package *) tp;
	int i = 0;

	//This is application specific. A robust hash table
	//would be nice, but this is a .c file. 
	//This hashTable indexes into all of the nodes this thread 
	//will ever see (id, node)
	int hash_size = (numNodes / numThreads) + 1 ;
	node **hashNodes = (node **) malloc(hash_size*sizeof(node *));
	for(i = 0; i < hash_size; i++) hashNodes[i] = globalList[numThreads * i + x->thread_id];

	//This hash hashes into the queue by height. 
	//It needs to be resizable :-(
	//OR JUST REALLY BIG!
	//Or we could use context clues to make a good guess
	//If we had them (and we could)
	int MAGIC_HEIGHT_GUESS = numNodes;
	node **hashHeight = (node **) malloc(sizeof(node *)*MAGIC_HEIGHT_GUESS);
	for(i = 0; i < hash_size; i++) hashHeight[i] = NULL;
	//I need some way to access the "previous" and "tail"
	//rather than increase the size of a node
	//I have decided to keep "head" and "tail" pointers
	//to every height. Which is still a little magical.
	node **hashHeightTail = (node **) malloc(sizeof(node *)*MAGIC_HEIGHT_GUESS);
	for(i = 0; i < hash_size; i++) hashHeightTail[i] = NULL;

	//methodize - also up? ^
	node *local_head = x->start_node;
	if(local_head != NULL) 
	{
		local_head->next = NULL;
		hashHeight[0] = local_head;
	}
	//end
	
	while(1)
	{
		node *curr = pop(&local_head, exploreDepth, &hashHeight, &hashHeightTail);
		//if(curr!= NULL) printf("popped: %d at %d\n", curr->id, curr->height);
		if(reduction(curr) == 1) break ;

		int my_height = 0;
		for(i = 0; i < exploreDepth && curr != NULL; i++)
		{
			if(curr->height > my_height) my_height = curr->height;
			explore(curr, curr->height + 1, x->thread_id);
			curr = curr->next;
		}

		pthread_barrier_wait(&barr);

		push(&local_head, x->thread_id, hashNodes, &hashHeight, &hashHeightTail);
	}

	return NULL;
}

//Query each thread for an unexplored node
//better stopping conditions & mechanisms exist.
int reduction(node *curr)
{
	if(curr != NULL)
	{
		global_flag = 0;
	}
	pthread_barrier_wait(&barr);
	int x = global_flag;
	pthread_barrier_wait(&barr);
	global_flag = 1;
	return x;
}

//dequeue exploreDepth nodes from our local queue
//NEED TO HANDLE UNCONNECTED GRAPHS!!!! TODO
node *pop(node **local_head_ref, int exploreDepth, node ***hashHeight_ref, node ***hashHeightTail_ref)
{
	node *local_head = *local_head_ref;
	node **hashHeight = *hashHeight_ref;
	node **hashHeightTail = *hashHeightTail_ref;
	if(local_head == NULL)
		return NULL;
	node *ret = local_head;
	node *last = NULL;
	int i; for(i = 0; i < exploreDepth && local_head != NULL; i++)
	{
//printf("popping: %d at %d\n", local_head->id, local_head->height);
		if(last != NULL && last->height != local_head->height)
		{
			hashHeight[last->height] = NULL;
			hashHeightTail[last->height] = NULL;
		}
		last = local_head;
		local_head = local_head->next;
	}

	if(last->next != NULL && last->next->height == last->height)
		hashHeight[last->height] = last->next;
	if(last != NULL) last->next = NULL;

	*hashHeightTail_ref = hashHeightTail;
	*hashHeight_ref = hashHeight;
	*local_head_ref = local_head;
	return ret;
}

//for each node we are exploring, observe its neighbors and
//aggregate them into one list.
void explore(node *curr, int height, int thread_id)
{
	while(curr != NULL)
	{
		edgeList *neighbors = curr->neighbors;
		//methodize
		while(neighbors != NULL)
		{
			node *neighbor = neighbors->neighbor;	
			if(neighbor->height > height)
			{
				new_node *new = (new_node *) malloc(sizeof(new_node));
				new->id = neighbor->id;
				new->height = height;
//printf("explored: %d at %d\n", new->id, height);
				//add to communication structure
				//This structure divides much more nicely AFL style
				new->next = Communication[mapping(new->id)][thread_id];
				Communication[mapping(new->id)][thread_id] = new;
				new->upTree = curr;
			}

			neighbors = neighbors->next;
		}
		//end
		curr = curr->next;
	}
}

//retrieve nodes from communication structure
//aggregate
//add locally
void push(node **local_head_ref, int thread_id, node **hashNodes, node ***hashHeight_ref, node ***hashHeightTail_ref)
{
	new_node **new = Communication[thread_id];	
	int i = 0;
	for(i = 0; i < numThreads; i++)
	{
		new_node *temp = new[i];
		while(temp != NULL)
		{
			add(local_head_ref, temp, hashNodes, hashHeight_ref, hashHeightTail_ref);
			temp = temp->next;
		}
		
		Communication[thread_id][i] = NULL;
	}
}

void add(node **local_head_ref, new_node *temp, node **hashNodes, node ***hashHeight_ref, node ***hashHeightTail_ref)
{
	int id = temp->id;
	int height = temp->height;
	node *new = hashNodes[threadHash(id)];
	if(new->height <= height)
		return ;

	node *local_head = *local_head_ref;
	node **hashHeight = *hashHeight_ref;
	node **hashHeightTail = *hashHeightTail_ref;

	new->height = height;
	new->upTree = temp->upTree;
	node *next = hashHeight[height];
	new->next = next;
	hashHeight[height] = new;
	if(local_head == next)
		local_head = new;

	//height always > 0
	//There should be a better management where
	//this tail balue is just tied to the next pointer
	//hashHeight[height]->next
	if(hashHeightTail[height-1] != NULL)
		hashHeightTail[height-1]->next = new;	

	if(hashHeightTail[height] == NULL)
		hashHeightTail[height] = new;

	*hashHeightTail_ref = hashHeightTail;
	*hashHeight_ref = hashHeight;
	*local_head_ref = local_head;
}

//A function which maps work to threads
int mapping(int id)
{
	return id % numThreads;
}

int threadHash(int x)
{
	return x / numThreads;
}

node** BuildGraph(char *filename)
{
	FILE *fd;
	fd = fopen(filename, "r");
	if(fd == NULL) {printf("Input File: %s could not be opened for reading\n", filename); exit(1);}

	char num_str[80];
	numNodes = atoi(fgets(num_str, 80, fd));
	node** graph = (node **) malloc(sizeof(node)*numNodes);

	int i = 0;
	globalList = (node **) malloc(numNodes*sizeof(node *));
	for(i = 0; i< numNodes; i++) globalList[i] = NULL;

	//how big this buffer needs to be is of some mild interest... and is currently over allocated
	int BUFF_SIZE = numNodes*numNodes;
	char *str = (char *) malloc(sizeof(char)*BUFF_SIZE);
	for(i = 0; i < numNodes; i++)
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
	node *new_node = hashNode(atoi(str), globalList);
	str = ++ptr;
	int flag = 0;
	while(flag == 0)
	{
		while(*ptr != ' ' && *ptr != '\n' && *ptr != EOF) ptr++;
		if(*ptr == '\n' || *ptr == EOF) flag = 1;
		*ptr = '\0';
		int edge = atoi(str);

		node *neighbor = hashNode(edge, globalList);
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
node *hashNode(int key, node **hashNodes)
{
/*
	if(hashNodes == NULL)
	{
		hashNodes = (node **) malloc(numNodes*sizeof(node *));
		int i = 0; for(; i< numNodes; i++) hashNodes[i] = NULL;
	}
*/
	if(hashNodes[key] == NULL)
	{
		//methodize
		hashNodes[key] = (node *) malloc(sizeof(node));
		hashNodes[key]->id = key;
		hashNodes[key]->neighbors = NULL;
		hashNodes[key]->height = INFINITY;
		//end
	}

	return hashNodes[key];
}

void init_globals(int numThreads)
{
	Communication = (new_node ***) malloc(sizeof(new_node **)*numThreads);
	int i = 0;
	for(i = 0; i < numThreads; i++)
		Communication[i] = (new_node **) malloc(sizeof(new_node *)*numThreads);
	pthread_barrier_init(&barr, NULL, numThreads);

	//make a better solution than this
	master_order = (int *) malloc(numNodes*sizeof(int));
	pthread_mutex_init(&master_lock, NULL);	
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
