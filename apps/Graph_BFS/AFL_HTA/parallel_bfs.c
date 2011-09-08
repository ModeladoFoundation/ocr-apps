#include "parallel_bfs.h"
#include "rmd_super_header.h"

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

	//TearDownGraph();

	//printBFS(stdout, traversal, graph, numNodes);
	return 0;
}

int* BFS(node **graph, int numNodes, int start, int numThreads)
{
	init_globals(numThreads);
	//pthread_t *threads = (pthread_t *) malloc(numThreads*sizeof(pthread_t));
	//Thread_Package *thread_package = (Thread_Package *) malloc(numThreads*sizeof(Thread_Package));
	pthread_t *threads = (pthread_t *) db_malloc(numThreads*sizeof(pthread_t), OFF_CHIP | REQUIRED, COREID_SELF);
	Thread_Package *thread_package = (Thread_Package *) db_malloc(numThreads*sizeof(Thread_Package), OFF_CHIP | REQUIRED, COREID_SELF);

	int i;
	//methodize
	int start_mapping = mapping(graph[start]->id);
	graph[start]->height = 0;
	//end
	
	for(i = 1; i < numThreads; i++)
	{
		thread_package[i].thread_id = i;
		thread_package[i].start_node = NULL;
		if(i == start_mapping)
			thread_package[i].start_node = graph[start];
		pthread_create(&threads[i], NULL, &BFS_worker, (void *) (&thread_package[i]));
	}

	thread_package[0].thread_id = 0;
	thread_package[0].start_node = NULL;
	if(0 == start_mapping)
		thread_package[0].start_node = graph[start];
	BFS_worker((void *) (&thread_package[0]));

	for(i = 1; i < numThreads; i++)
		pthread_join(threads[i], NULL);

	db_free_one(threads);
	db_free_one(thread_package);

	return NULL;
}

void *BFS_worker(void *tp)
{
	Thread_Package *x = (Thread_Package *) tp;
	int i = 0;

	//change me to support more blocks TODO
	initmem_rmd_thread(find_core_id(x->thread_id));

	data_block_t comm_block;
	DB_ALLOC(&comm_block, sizeof(new_node *)*numThreads, REQUIRED | LOCAL, COREID_SELF);
	DB_MEM(&(Communication[x->thread_id]), comm_block);

	//This is application specific. A robust hash table
	//would be nice, but this is a .c file. 
	//This hashTable indexes into all of the nodes this thread 
	//will ever see (id, node)
	int hash_size = (numNodes / numThreads) + 1 ;
	//node **hashNodes = (node **) malloc(hash_size*sizeof(node *));
	node **hashNodes = (node **) db_malloc(hash_size*sizeof(node *), LOCAL | REQUIRED, COREID_SELF);
	for(i = 0; i < hash_size; i++) hashNodes[i] = globalList[numThreads * i + x->thread_id];

	//This hash hashes into the queue by height. 
	//It needs to be resizable :-(
	//OR JUST REALLY BIG!
	//Or we could use context clues to make a good guess
	//If we had them (and we could)
	int MAGIC_HEIGHT_GUESS = numNodes;
	//node **hashHeight = (node **) malloc(sizeof(node *)*MAGIC_HEIGHT_GUESS);
	node **hashHeight = (node **) db_malloc(sizeof(node *)*MAGIC_HEIGHT_GUESS, LOCAL | REQUIRED, COREID_SELF);
	for(i = 0; i < hash_size; i++) hashHeight[i] = NULL;
	//I need some way to access the "previous" and "tail"
	//rather than increase the size of a node
	//I have decided to keep "head" and "tail" pointers
	//to every height. Which is still a little magical.
	//node **hashHeightTail = (node **) malloc(sizeof(node *)*MAGIC_HEIGHT_GUESS);
	node **hashHeightTail = (node **) db_malloc(sizeof(node *)*MAGIC_HEIGHT_GUESS, LOCAL | REQUIRED, COREID_SELF);
	for(i = 0; i < hash_size; i++) hashHeightTail[i] = NULL;

	//methodize - also up? ^
	node *LOCAL_head = x->start_node;
	if(LOCAL_head != NULL) 
	{
		LOCAL_head->next = NULL;
		hashHeight[0] = LOCAL_head;
	}
	//end
	
	while(1)
	{
		node *curr = pop(&LOCAL_head, exploreDepth, &hashHeight, &hashHeightTail);
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

		push(&LOCAL_head, x->thread_id, hashNodes, &hashHeight, &hashHeightTail);
	}

	db_free_one(hashNodes);
	//db_free_one(hashHeight);
	//db_free_one(hashHeightTail);
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

//dequeue exploreDepth nodes from our LOCAL queue
//NEED TO HANDLE UNCONNECTED GRAPHS!!!! TODO
node *pop(node **LOCAL_head_ref, int exploreDepth, node ***hashHeight_ref, node ***hashHeightTail_ref)
{
	node *LOCAL_head = *LOCAL_head_ref;
	node **hashHeight = *hashHeight_ref;
	node **hashHeightTail = *hashHeightTail_ref;
	if(LOCAL_head == NULL)
		return NULL;
	node *ret = LOCAL_head;
	node *last = NULL;
	int i; for(i = 0; i < exploreDepth && LOCAL_head != NULL; i++)
	{
//printf("popping: %d at %d\n", LOCAL_head->id, LOCAL_head->height);
		if(last != NULL && last->height != LOCAL_head->height)
		{
			hashHeight[last->height] = NULL;
			hashHeightTail[last->height] = NULL;
		}
		last = LOCAL_head;
		LOCAL_head = LOCAL_head->next;
	}

	if(last->next != NULL && last->next->height == last->height)
		hashHeight[last->height] = last->next;
	if(last != NULL) last->next = NULL;

	*hashHeightTail_ref = hashHeightTail;
	*hashHeight_ref = hashHeight;
	*LOCAL_head_ref = LOCAL_head;
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
				int owner = mapping(neighbor->id);
				//TODO DON"T DO THIS ALLOCATION OVER AND OVER AGAIN
				new_node *new = (new_node *) db_malloc(sizeof(new_node), REQUIRED | BLOCK_LOCAL, find_core_id(owner));
				//new_node *new = (new_node *) malloc(sizeof(new_node));
				new->id = neighbor->id;
				new->height = height;
//printf("explored: %d at %d\n", new->id, height);
				//add to communication structure
				//This structure divides much more nicely AFL style
				new->next = Communication[owner][thread_id];
				Communication[owner][thread_id] = new;
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
//add LOCALly
void push(node **LOCAL_head_ref, int thread_id, node **hashNodes, node ***hashHeight_ref, node ***hashHeightTail_ref)
{
	new_node **new = Communication[thread_id];	
	int i = 0;
	for(i = 0; i < numThreads; i++)
	{
		new_node *temp = new[i];
		while(temp != NULL)
		{
			add(LOCAL_head_ref, temp, hashNodes, hashHeight_ref, hashHeightTail_ref);
			temp = temp->next;
		}
		
		Communication[thread_id][i] = NULL;
	}
}

void add(node **LOCAL_head_ref, new_node *temp, node **hashNodes, node ***hashHeight_ref, node ***hashHeightTail_ref)
{
	int id = temp->id;
	int height = temp->height;
	node *new = hashNodes[threadHash(id)];
	if(new->height <= height)
		return ;

	node *LOCAL_head = *LOCAL_head_ref;
	node **hashHeight = *hashHeight_ref;
	node **hashHeightTail = *hashHeightTail_ref;

	new->height = height;
	new->upTree = temp->upTree;
	node *next = hashHeight[height];
	new->next = next;
	hashHeight[height] = new;
	if(LOCAL_head == next)
		LOCAL_head = new;

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
	*LOCAL_head_ref = LOCAL_head;
}

int split_factor = 3;
//A function which maps work to threads
int mapping(int id)
{
	if(id / split_factor > numThreads) printf("mapping: %d\tsplit_factor: %d\n", (id / split_factor), split_factor);
	return id / split_factor;
}

int threadHash(int x)
{
	return x / numThreads;
}

int magically_decide_number_of_chips_based_on_this_specific_algorithms_memory_usage_function_of_badness()
{
	int maxEdgeGuess = numNodes; //TODO
	int LOCAL_total = sizeof(node)*numNodes + sizeof(node *)*((numNodes / numThreads + 1) + numNodes*2) + sizeof(new_node)*maxEdgeGuess + sizeof(edgeList) * maxEdgeGuess;
	printf("numNodes: %d\tLOCAL_total: %d\n", numNodes, LOCAL_total);
	int block_total = sizeof(node)*numNodes + sizeof(new_node **);

	int LOCAL_req = (LOCAL_total / engineHeap) / (nXEs * nBlocks);
	int block_req = block_total / (blockHeap * nBlocks);
	if(LOCAL_req == 0) LOCAL_req = 1;

	printf("LOCAL req: %d\tblock_req: %d\n", LOCAL_req, block_req);
	
	return LOCAL_req > block_req ? LOCAL_req : block_req;
}

node** BuildGraph(char *filename)
{
	FILE *fd;
	fd = fopen(filename, "r");
	if(fd == NULL) {printf("Input File: %s could not be opened for reading\n", filename); exit(1);}

	char num_str[80];
	numNodes = atoi(fgets(num_str, 80, fd));
	
	//change the number of chips to request TODO

	//nChips = magically_decide_number_of_chips_based_on_this_specific_algorithms_memory_usage_function_of_badness();
	
	nChips = ((numThreads-1) / (nXEs*nBlocks)) + 1;

	split_factor = numNodes / numThreads + 1;
	setup_rmd_memory_global(nChips, nBlocks, nCEs, nXEs);
	initmem_rmd_thread(find_core_id(0));

	//This graph should be "multicasted" across the available block memory
	//node** graph = (node **) malloc(sizeof(node)*numNodes);
	node** graph = (node **) db_malloc(sizeof(node)*numNodes, OFF_CHIP, COREID_SELF);

	int i = 0;
	//This organization of the nodes lives in DRAM. I use too much bookkeeping data. 
	//globalList = (node **) malloc(numNodes*sizeof(node *));
	globalList = (node **) db_malloc(numNodes*sizeof(node *), OFF_CHIP | REQUIRED, COREID_SELF);
	for(i = 0; i< numNodes; i++) globalList[i] = NULL;

	//how big this buffer needs to be is of some mild interest... and is currently over allocated
	int BUFF_SIZE = numNodes*sizeof(int)*sizeof(int);
	//char *str = (char *) malloc(sizeof(char)*BUFF_SIZE);
	char *str = (char *) db_malloc(sizeof(char)*BUFF_SIZE, OFF_CHIP | REQUIRED, COREID_SELF);
	for(i = 0; i < numNodes; i++)
	{
//printf("i: %d\n", i);
		fgets(str, BUFF_SIZE, fd);
		graph[i] = parseEdgeList(str);
	}

	//free(str);
	db_free_one(str);
	fclose(fd);

	printf("Graph Built Successfully\n");
	
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
		//edgeList* neighbors = (edgeList *) malloc(sizeof(edgeList));
		edgeList* neighbors = (edgeList *) db_malloc(sizeof(edgeList), LOCAL | REQUIRED, find_core_id(mapping(atoi(str))));
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
		//hashNodes[key] = (node *) malloc(sizeof(node));
		hashNodes[key] = (node *) db_malloc(sizeof(node), LOCAL | REQUIRED, find_core_id(mapping(key)));
		hashNodes[key]->id = key;
		hashNodes[key]->neighbors = NULL;
		hashNodes[key]->height = INFINITY;
		//end
	}

	return hashNodes[key];
}

void init_globals(int numThreads)
{

//	These must be created at a per thread level, but we need a way for all the threads to know where they are.
//	AKA desperately needs multicasting FIXME
	Communication = (new_node ***) db_malloc(sizeof(new_node **)*numThreads, OFF_CHIP, COREID_SELF);

/*	int i = 0;
	for(i = 0; i < numThreads; i++)
		Communication[i] = (new_node **) malloc(sizeof(new_node *)*numThreads);
*/

	pthread_barrier_init(&barr, NULL, numThreads);

	//make a better solution than this
	//master_order = (int *) malloc(numNodes*sizeof(int));
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
