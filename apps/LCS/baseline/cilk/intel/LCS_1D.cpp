/*
 @Author: Jesmin Jahan Tithi, jtithi@cs.stonybrook.edu, shraban03@gmail.com.
 This is a recursive divide and conquer LCS program that uses linear space to compute the length of the longest common subsequence.
 */


// C libraries
#include <stdio.h>
#include <math.h>

// C++ libraries
#include <iostream>
#include <algorithm>

// Cilk libraries
#include<cilk/cilk.h>
#include<cilk/cilk_api.h>

// User defined libraries
#include "cilktime.h" // timing library


// Other macros
#define ALIGN 64
#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)

using namespace std;

#define GAP_PENALTY 0 //for LCS 0, for edit distance 1 and use min, for Smith waterman depends on the blosum matrix

int *lcs_score;            // Stores LCS score
int *S, *T;               // Stores for string 1 and 2
int base=1;
long int N;                // Size of the strings (assumption both strings are of same size)
long int Nplus1;

/*Generate Random Input*/
void genRandInput(int *X, int *Y, int n)
{
    char a = 'A';
    for(int i =0; i<n ; i ++)
    {
        X[i]=rand()%4+a;
    }
    for(int i = 0; i<n ; i ++)
    {
        Y[i]=rand()%4+a;
    }
}


void inline serial_lcs_2D(int* X, long int xi, long int xj, long int n){

    // Finish computation
    // Conditions to handle non powers of 2.
    long int endi = (xi + n > N) ? (Nplus1  - xi) : n;
    long int endj = (xj + n > N) ? (Nplus1  - xj) : n;

    endi = xi + endi;
    endj = xj + endj;

    for (int i = xi; i < endi; i++) {
        for(int j = xj; j <endj;j++)
        {
            long int idx = (i-1)*Nplus1;
            //Optimize
            int del = X[idx+j];
            int ins = X[idx + Nplus1 + (j - 1)];
            int del_ins = (del > ins) ? del + GAP_PENALTY:ins + GAP_PENALTY;
            int match = X[idx  + (j - 1)] + (S[i] != T[j]);
            X[idx + Nplus1 +j]= MAX(match, del_ins);
        }
    }
    // Copy back to original location
}

void inline serial_lcs_1D(int* X, long int xi, long int xj, long int n){


    long int endi = (xi + n > N) ? (Nplus1  - xi) : n;
    long int endj = (xj + n > N) ? (Nplus1  - xj) : n;

    // Diagonal by diagonal computation.

    int nx2 = 2 * endi - 1;
    int n_minus1 = endi - 1;

    // First half of the diagonals
    for (int t = 0; t < endi; t++)
    {
        for(int i = 0; i <= t;i++)
        {
            int j = t - i;
            long int idx = (N+(xj+j-xi-i));
            //Optimize
            int del = X[idx - 1];
            int ins = X[idx + 1];
            int del_ins = (del > ins) ? (del + GAP_PENALTY):(ins + GAP_PENALTY);
            int match = X[idx] + (S[xi+i] != T[xj+j]);
            X[idx]= MAX(match, del_ins);

        }

    }
    // Second half of the diagonals
    for (int t = endi; t < nx2; t++) {
        for(int i = n_minus1; i >= (t - n_minus1);i--)
        {
            int j = t - i;
            long int idx = (N+(xj+j-xi-i));
            //Optimize
            int del = X[idx - 1];
            int ins = X[idx + 1];
            int del_ins = (del > ins) ? (del + GAP_PENALTY):(ins + GAP_PENALTY);
            int match = X[idx] + (S[xi+i] != T[xj+j]);
            X[idx]= MAX(match, del_ins);
        }
    }

    return;
}
void LCS_orig(long int xi, long int xj, long int n) {
    if(n<=base)
    {

        serial_lcs_1D(lcs_score, xi, xj, n);
        return;
    }
    else
    {
        long int nn = n >> 1;


        LCS_orig(xi,    xj,    nn); //x11
        cilk_spawn LCS_orig(xi, xj+nn, nn); //x12
        LCS_orig(xi+nn, xj,    nn); //x21
        cilk_sync;
        LCS_orig(xi+nn, xj+nn, nn); //x22
    }
}

int main(int argc, char* argv[]){
    int p = __cilkrts_get_nworkers(); // number of threads/processors.
    if(argc<1)
    {
        cout<<"USAGE: please run the program as follows:\n"
        <<"executable input_string_length number_of_cores basecase cutoff_for_2way\n";
    }
    if(argc > 1)
    {
        N = atol(argv[1]);
    }
    if(argc > 2)
    {
        p = atoi(argv[2]);
        __cilkrts_set_param("nworkers", argv[2]);
    }
    if(argc > 3)
    {
        base = atoi(argv[3]);
        if(N <= base) base = N;
    }
    // Fix base case as needed


    /*if(N/sqrt(p) > 1)
     base = N/sqrt(p);
     */
    // Actual allocation size
    Nplus1 = N + 1;

    // Allocate LCS table memory and the strings.
    S = ( int * ) _mm_malloc(Nplus1 *  sizeof( int ), ALIGN );
    T = ( int * ) _mm_malloc(Nplus1 *  sizeof( int ), ALIGN );

    S[0]=T[0]=32;
    genRandInput(S, T, Nplus1);

    // Linear space array.
    lcs_score = ( int * ) _mm_malloc( (2 * Nplus1) *  sizeof( int ), ALIGN );

    if(lcs_score==NULL)
    {
        _mm_free(lcs_score);
        cout<<"Allocation Failed in lcs_score"<<endl;
        exit(1);
    }
    // Initialize the matrix.
    cilk_for ( int i = 0; i < Nplus1; i++ )
    {
        lcs_score[N - i] = i;
    }
    cilk_for ( int j = 0; j < Nplus1; j++ )
    {
        lcs_score[N + j] = j;
    }

    int NN = 2; while(NN < N) NN = NN <<1;

	unsigned long start;
	start = cilk_getticks();

	LCS_orig(1, 1, NN);

	long end = cilk_getticks();
	printf("runtime: %f\n",cilk_ticks_to_seconds(end-start));

	#ifdef VERIFY
	int *D = ( int * ) _mm_malloc( Nplus1 * Nplus1 * sizeof( int ), ALIGN );
	if(D==NULL)
	{
		_mm_free(D);
		cout<<"Allocation Failed in lcs_score"<<endl;
		exit(1);
	}
	// Initialize the matrix.
	cilk_for ( int i = 0; i < Nplus1; i++ )
	{
		D[i*Nplus1] = i;
	}
	cilk_for ( int j = 0; j < Nplus1; j++ )
	{
		D[j] = j;
	}


	serial_lcs_2D(D, 1, 1, NN);



	#ifdef PRINT_SCORE
	for(int i = 0; i < Nplus1; i++)
	{
		for(int j = 0; j < Nplus1; j++)
		{

			cout<<lcs_score[i*Nplus1+j] <<" ";
		}
		cout<<"\n";
	}
    for(int i = 0; i < 2*Nplus1; i++)
	{
		cout<<D[i] <<" ";
	}
	cout<<"\n";
	#endif

	assert(D[N*Nplus1+N]==lcs_score[N]);
	#endif


    _mm_free(S);
    _mm_free(T);
    _mm_free(lcs_score);


    cout<<endl;
    return 0;
}

