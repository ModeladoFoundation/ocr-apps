#include "parallel_bfs.h"
#include "BinParse.c"

int main(int argc, char *argv[])
{
	setup_rmd_memory_global(nChips, nBlocks, nCEs, nXEs);
	initmem_rmd_thread(gen_core_id(0,0,0));

	struct timeval startTime, endTime;
	int conversion = 1000000;

	numThreads = 2;
	char *file = (char *) "/home/gibbons/APPS/BFS/Newput/default.txt";
	if(argc > 1) file = argv[1];
	if(argc > 2) numThreads = atoi(argv[2]);

	int bin = 0;
	printf("pre loop\n");
	while(file[bin] != '\0') bin++;
	if(strncmp(&(file[bin-4]), ".bin", 4) == 0) bin = 0;
	printf("bin: %d\n", bin);
	gettimeofday(&startTime, NULL);
	if(!bin) BinParse(file);
	else BuildGraph(file);
	gettimeofday(&endTime, NULL);
	double tS = startTime.tv_sec*conversion + (startTime.tv_usec);
	double tE = endTime.tv_sec*conversion + (endTime.tv_usec);
	printf("Time to build graph: %f seconds\n", (tE-tS)/conversion);
	
	gettimeofday(&startTime, NULL);
	BFS(numNodes, 1, numThreads);
	gettimeofday(&endTime, NULL);

	tS = startTime.tv_sec*conversion + (startTime.tv_usec);
	tE = endTime.tv_sec*conversion + (endTime.tv_usec);
	printf("Time to run BFS: %f seconds\n", (tE-tS)/conversion);

	//printDP();
	return 0;
}

int* BFS(int numNodes, int start, int numThreads)
{
	init_globals(numThreads);
	pthread_t *threads = (pthread_t *) db_quick_malloc(numThreads*sizeof(pthread_t));
	Thread_Package *thread_package = (Thread_Package *) db_quick_malloc(numThreads*sizeof(Thread_Package));

	int i = 0;
	//methodize
	int start_mapping = mapping(start);
	glbl_DepthParent[2*start] = 0;
	glbl_DepthParent[2*start+1] = -1;
	//end
	
	for(; i < numThreads; i++)
	{
		thread_package[i].thread_id = i;
		thread_package[i].start_id = -1;
		if(i == start_mapping)
			thread_package[i].start_id = start;
		pthread_create(&threads[i], NULL, &BFS_worker, (void *) (&thread_package[i]));
	}

	for(i = 0; i < numThreads; i++)
		pthread_join(threads[i], NULL);

	return NULL;
}

void *BFS_worker(void *tp)
{
	Thread_Package *x = (Thread_Package *) tp;

	initmem_rmd_thread(gen_core_id(0, 0, x->thread_id % (nXEs + nCEs)));

	//boundary vertices.
	int *current; //new local_head

	int current_size = 0; //new local_tail

	if(x->start_id != -1) 
	{
		current_size = 1;
		current = (int *) db_quick_malloc(sizeof(int));
		current[0] = x->start_id;
	}

	while(1)
	{
		//if(curr!= NULL) printf("popped: %d at %d\n", curr->id, curr->height);
		if(reduction(current_size) == 1) break ;

		int j = 0; for(j = 0; j < current_size; j++)
		{
			explore(current[j], glbl_DepthParent[2*(current[j])] + 1, x->thread_id);
		}

		pthread_barrier_wait(&barr);

		current_size = push(&current, x->thread_id);
	}

	return NULL;
}

//Query each thread for an unexplored node
//better stopping conditions & mechanisms exist.
int reduction(int current_size)
{
	if(current_size != 0)
	{
		global_flag = 0;
	}
	pthread_barrier_wait(&barr);
	int x = global_flag;
	pthread_barrier_wait(&barr);
	global_flag = 1;
	return x;
}

//for each node we are exploring, observe its neighbors and
//aggregate them into one list.
void explore(int id, int height, int thread_id)
{
//int x = 0;
//printf("current explorer: %d #: %d\n", curr->id, x++);
	int numNeighbors = glbl_Adjacency[id][0];
//printf("numNeighbors: %d\n", numNeighbors);
	int i; for(i = 1; i <= numNeighbors; i++)
	{
		int neighbor_id = glbl_Adjacency[id][i];
		int byte = neighbor_id / bitsPerByte;
		int bit = neighbor_id % bitsPerByte;
		if((glbl_VIS[byte] & (1 << bit)) == 0)
		{
			glbl_VIS[byte] |= 1 << bit;
			if(glbl_DepthParent[neighbor_id*2] > height)
			{
				new_node *new = (new_node *) db_quick_malloc(sizeof(new_node));
				new->id = neighbor_id;
				glbl_DepthParent[neighbor_id*2] = height;
//printf("explored: %d at %d\n", new->id, height);
				//add to communication structure
				//This structure divides much more nicely AFL style
				new->next = Communication[mapping(new->id)][thread_id];
				Communication[mapping(new->id)][thread_id] = new;
				Communication_count[mapping(new->id)][thread_id]++;
				glbl_DepthParent[2*neighbor_id + 1] = id;
			}
		}
	}
}

//retrieve nodes from communication structure
//aggregate
//add locally
int push(int **current_ref, int thread_id)
{
	new_node **new = Communication[thread_id];	
	int i = 0;
	int sum = 0;
	for(i = 0; i < numThreads; i++)
		sum += Communication_count[thread_id][i];

	*current_ref = (int *) db_quick_malloc(sizeof(int)*sum);

	int index = 0;
	for(i = 0; i < numThreads; i++)
	{
		while(new[i] != NULL)
		{
			(*current_ref)[index] = new[i]->id;
			new[i] = new[i]->next;
			index++;
		}
		
		Communication[thread_id][i] = NULL;
		Communication_count[thread_id][i] = 0;
	}

	return sum;
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

void BuildGraph(char *filename)
{
	FILE *fd;
	fd = fopen(filename, "r");
	if(fd == NULL) {printf("Input File: %s could not be opened for reading\n", filename); exit(1);}

	char num_str[80];
	numNodes = atoi(fgets(num_str, 80, fd));
	//printf("numNodes: %d\n", numNodes);
	int i = 0;

	glbl_Adjacency = (int **) db_quick_malloc(numNodes*sizeof(int *));

	//how big this buffer needs to be is of some mild interest... and is currently over allocated
	int BUFF_SIZE = numNodes*sizeof(int)*sizeof(int);
	char *str = (char *) db_quick_malloc(sizeof(char)*BUFF_SIZE);
	for(i = 0; i < numNodes; i++)
	{
		fgets(str, BUFF_SIZE, fd);
		parseEdgeList(str, i, BUFF_SIZE);
	}
	//return graph;
}

void parseEdgeList(char *str, int key, int BUFF_SIZE)
{
	int length = 0;
	char *ptr = str;
	char *temp = (char *) db_quick_malloc(BUFF_SIZE*sizeof(char));
	while(ptr[length] != ' ') length++;
	strncpy(temp, ptr, length);
	temp[length] = '\0';

	int neighbor_count = atoi(temp);
	glbl_Adjacency[key] = (int *) db_quick_malloc((neighbor_count+1)*sizeof(int));
	glbl_Adjacency[key][0] = neighbor_count;
	
	ptr = &(ptr[length+1]);
	int i; for(i=1; i <= neighbor_count; i++)
	{
		length= 0;
		while(ptr[length] != ' ' && ptr[length] != '\n' && ptr[length] != EOF) length++;
		strncpy(temp, ptr, length);
		temp[length] = '\0';
		int edge = atoi(temp);

		glbl_Adjacency[key][i] = edge;
	
		ptr = &(ptr[length+1]);
	}
}

void init_globals(int numThreads)
{
	Communication = (new_node ***) db_quick_malloc(sizeof(new_node **)*numThreads);
	Communication_count = (int **) db_quick_malloc(sizeof(int *)*numThreads);
	int i = 0;
	for(i = 0; i < numThreads; i++)
	{
		Communication[i] = (new_node **) db_quick_malloc(sizeof(new_node *)*numThreads);
		Communication_count[i] = (int *) db_quick_malloc(sizeof(int)*numThreads);
		int j = 0; for(j = 0; j < numThreads; j++)
		{
			Communication_count[i][j] = 0;
			Communication[i][j] = NULL;
		}
	}
	pthread_barrier_init(&barr, NULL, numThreads);

	//make a better solution than this
	master_order = (int *) db_quick_malloc(numNodes*sizeof(int));
	pthread_mutex_init(&master_lock, NULL);	

	int size = numNodes / (sizeof(char)*bitsPerByte);
	if (numNodes % (sizeof(char)*bitsPerByte)) size++;
	glbl_VIS = (char *) db_quick_malloc(size);
	for(i = 0; i < size / sizeof(char); i++) glbl_VIS[i] = 0;

	glbl_DepthParent = (int *) db_quick_malloc(2*sizeof(int)*numNodes); // one for depth, one for parent
	//maybe unnecessary initialization
	for(i = 0; i < numNodes; i++){
		glbl_DepthParent[2*i] = INFINITY;
		glbl_DepthParent[2*i+1] = -2;
	}
}

void printDP()
{
	int i;
	for(i = 0; i < numNodes; i++)
	{
		printf("i: %d\tp: %d\td: %d\t\n", i, glbl_DepthParent[2*i+1], glbl_DepthParent[2*i]);
	}
}
