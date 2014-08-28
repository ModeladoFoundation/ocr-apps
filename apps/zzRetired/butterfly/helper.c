int getNumPoints() {return numPoints;}

/*
 *  *      prints all the points in a list
 *   */
void printPoints(point* a, int N)
{
        int k;
        for(k = 0; k < N; k++)
        {
                printPoint(a[k]);
        }
}

void printPoint(point p)
{
        printf("%f + %fi\n", p.real, p.imaginary);
}

/*
 *  *      add two points (operators are not overloadable)
 *   */
point plus(const point a, const point b)
{
        point x;
        x.real = a.real + b.real;
        x.imaginary = a.imaginary + b.imaginary;
        return x;
}

/*
 *  *      subtract two points (operators are not overloadable)
 *   */
point minus(const point a, const point b)
{
        point x;
        x.real = a.real - b.real;
        x.imaginary = a.imaginary - b.imaginary;
        return x;
}

point mult(const complex double a, const point b)
{
        point x;
        x.real = (b.real * creal(a)) - (b.imaginary * cimag(a));
        x.imaginary = (b.imaginary * creal(a)) + (b.real * cimag(a));
        return x;
}

void invert(point *fft, int N)
{
	int k = 0;
	for(; k < N; k++)
	{
		fft[k].real /= (double) N;
		fft[k].imaginary /= (double) N;
	}
}

/**
 *  *      Precomputes the twiddle factors
 *   */
int twiddle_verse = -42;
complex double* buildTwiddle(int numPoints, int inverse)
{
		if(twiddle == NULL)
		{
        	twiddle = (complex double *) malloc(numPoints*sizeof(complex double)/2);
		}

		if(inverse == twiddle_verse) return twiddle;

        int k;
        for(k = 0; k < numPoints/2; k++)
        {
                twiddle[k] = cexp(((double) inverse)*-2.0*pi*I*((double) k)/((double) numPoints));
        		//printf("%d %f %f", k, creal(twiddle[k]), cimag(twiddle[k]));
		}

		twiddle_verse = inverse;
        return twiddle;
}

point* copyPoints(point* a, int numPoints, int stride)
{
	point *b = (point *) malloc(numPoints*sizeof(point));
	int k = 0;
	int count  = 0;
	for(; k < numPoints; k += stride)
	{
		b[count].real = a[k].real;
		b[count].imaginary = a[k].imaginary;
		count++;
	}

	return b;
}

int checkPoints(point* a, point* b, int numPoints)
{
	int k = 0;
	for(; k < numPoints; k++)
	{
		if((a[k].real - b[k].real) > .001 || (a[k].imaginary - b[k].imaginary) > .001
		|| (a[k].real - b[k].real) < -.001 || (a[k].imaginary - b[k].imaginary) < -.001)
		{
			printf("First differ at index: %d\n", k);
			printPoint(a[k]);
			printPoint(b[k]);

			if(a[k].real != b[k].real) printf("real; %f\n", a[k].real - b[k].real);
			if(a[k].imaginary != b[k].imaginary) printf("imaginary: %f\n", a[k].imaginary - b[k].imaginary);
			return 0;
		}
	}

	return 1;
}

/*
 * 	Takes in a file where each line holds two points
 * 	one real and one complex
 * 	returns a 2xn array, where array[0][index] is the real component and
 * 	array[1][index] the imaginary component.
 * 	stored in the "point" datastructure
 */
point* buildDataPoints(char *file)
{
	int BUFF_SIZE = 80;
	FILE *fd;
	fd = fopen(file, "r");
	if(fd==NULL) {printf("Input File: %s could not be opened for reading\n", file); exit(0); }

	//set global
	numPoints = countLines(fd, BUFF_SIZE);
	point *dataPoints = (point *) malloc(numPoints*sizeof(point));

	char *str = (char *) malloc(BUFF_SIZE*sizeof(char));
	int index = 0, start = 0, end = 0;

	char num[40] = "\0";
	while(!feof(fd))
	{
		if(NULL == fgets(str, BUFF_SIZE, fd)) break;
		if(feof(fd)) break ;
		while(str[end] != ' ') {
			num[end] = str[end];
			end++;
		}
		num[end] = '\0';
		dataPoints[index].real = atof(num);
		start = end;
		end = 0;
		while(str[start+end] != '\0') {
			num[end] = str[start+end];
			end++;
		}
		num[start+end] = '\0';
		dataPoints[index].imaginary = atof(num);
		index++;
		end = 0;
	}

	free(str);
	fclose(fd);
	return dataPoints;
}

//replace Input/x as Output/x
char *outfileName(char *file)
{
	int k = 1;
	int i = 0;
	do {
		i++; k++;
	} while(file[i] != '\0');
	k++;

	char *out = (char *) malloc(k*sizeof(char));
	out[0] = 'O';
	out[1] = 'u';
	out[2] = 't';
	int j = 1;
	do {
		j++;
		out[j+1] = file[j];
	} while(file[j] != 0 && file[j] != ' ' && file[j] != '\n');

	if(j+1 != k-1) printf("j+1: %d k-1: %d\n", j+1, k-1);

	printf("outfile: %s\n", out);
	return out;
}

//out the transformed points
void writeData(point *dataPoints, char *fileName)
{
	FILE *fd = fopen(fileName, "w");
	int k = 0;
	for(; k < numPoints-1; k++)
		fprintf(fd, "%f %f\n", dataPoints[k].real, dataPoints[k].imaginary);
	fprintf(fd, "%f %f", dataPoints[k].real, dataPoints[k].imaginary);

	fclose(fd);
	free(fileName);
}

//count the lines in the file and return the descriptor to the beginning
//file should already be open and is not closed
int countLines(FILE *fd, int BUFF_SIZE)
{
	char* str = (char *) malloc(BUFF_SIZE*sizeof(char));
	int numPoints = -1;
	while(!feof(fd))
	{
		numPoints++;
		fgets(str, BUFF_SIZE, fd);
	}

	rewind(fd);
	free(str);

	return numPoints;
}

//free four inputs and twiddle. just for visual.
void freeAll(point *a, point *b, point *c, point *d)
{
	free(a);
	free(b);
	free(c);
	free(d);
	free(twiddle);
}

//the parallel version only uses
//threads that are powers of two
//to make it artificially nicer
int flatten(int numThreads)
{
	if(numThreads < 2) return 1;
	int count = 0 ;
	while(numThreads != 1)
	{
		numThreads /=2;
		count++;
	}
	while(count > 0)
	{
		numThreads *= 2;
		count--;
	}

	return numThreads;
}

int logTwo(int x)
{
	if(x < 1) return -1;
	int ret = 0;
	while(x != 1)
	{
		x /= 2;
		ret++;
	}

	return ret;
}

int powTwo(int k)
{
	int ret = 1;
	while(k > 0)
	{
		ret *= 2;
		k--;
	}

	return ret;
}

#include "dataset.h"
