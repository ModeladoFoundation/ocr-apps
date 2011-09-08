#include <stdio.h>
#include <stdlib.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

void BinParse(char *fileName)
{
	char X;
	FILE *fp;
	int binary_file=1;
	fp = fopen(fileName, "rb");

	fread(&X, sizeof(char), 1, fp); if (X != 'P') binary_file = 0;
	fread(&X, sizeof(char), 1, fp); if (X != 'C') binary_file = 0;
	fread(&X, sizeof(char), 1, fp); if (X != 'L') binary_file = 0;

	fread(&X, sizeof(char), 1, fp); if (X != '9') binary_file = 0;
	fread(&X, sizeof(char), 1, fp); if (X != '5') binary_file = 0;
	fread(&X, sizeof(char), 1, fp); if (X != '1') binary_file = 0;
	fread(&X, sizeof(char), 1, fp); if (X != '2') binary_file = 0;
	fread(&X, sizeof(char), 1, fp); if (X != '3') binary_file = 0;

	if (!binary_file)
	{
		printf("Bad input file ... bailing\n");
		return ;
	}

	int glbl_number_of_vertices;
	long long int glbl_number_of_edges;
	long long int number_of_integers;

	fread(&glbl_number_of_vertices, sizeof(int), 1, fp);
	fread(&glbl_number_of_edges, sizeof(long long int), 1, fp);
	fread(&number_of_integers, sizeof(long long int), 1, fp);

	numNodes = glbl_number_of_vertices;

	printf("glbl_number_of_vertices = %d ::: glbl_number_of_edges = %lld\n", glbl_number_of_vertices, glbl_number_of_edges);
	printf("Total memory needed = %lld\n", number_of_integers*sizeof(int));
	glbl_Adjacency = (int **)malloc(glbl_number_of_vertices*sizeof(int*));
	int *temp_memory  = (int *)malloc(number_of_integers*sizeof(int));
	
	long long int sum_of_neighbors = 0;

	fseek(fp, 28, SEEK_SET); // reset fp to the point where the non NUMA_AWARE code would be
        fread(temp_memory, number_of_integers, sizeof(int), fp);

	long long int max_neighbors = 0;
	long long int min_neighbors = glbl_number_of_edges + 1;

	size_t i; for(i=0; i<glbl_number_of_vertices; i++)
	{
		glbl_Adjacency[i] = temp_memory;
		temp_memory += (1+glbl_Adjacency[i][0]);
		//Count[glbl_Adjacency_Socket0[i][0]]++;
		sum_of_neighbors += glbl_Adjacency[i][0];
		max_neighbors = MAX(max_neighbors, glbl_Adjacency[i][0]);
		min_neighbors = MIN(min_neighbors, glbl_Adjacency[i][0]);
	}
	printf("Read graph: max_neighbors is %lld, min_neighbors is %lld\n", 
		max_neighbors, min_neighbors);

	if ( sum_of_neighbors != glbl_number_of_edges) 
	{
		printf("sum_of_neighbors = %lld ::: glbl_number_of_edges = %lld\n", sum_of_neighbors, glbl_number_of_edges);
		return ;
	}

	printf("Success!\n");
}
