/*
 * Utility to convert input to binary format
 */

#include <stdlib.h>
#include <stdio.h>


int main(int argc, const char *argv[]) {
    int i;

    if(argc != 4) {
        printf("Usage: %s numoffset size datafile\n", argv[0]);
        return 1;
    }

    FILE *in, *out;
    int size = -1;

    size = atoi(argv[2]);

    in = fopen(argv[3], "r");
    out = fopen("binInput.bin", "wb");
    if(!in || !out) {
        printf("Cannot open input or output files.\n");
        return 1;
    }
    unsigned long int *A = (unsigned long int *) malloc (sizeof(unsigned long int)*size);

    while(fscanf(in, "%d\n", &A[i])!=EOF){
        i++;
    }
    fwrite(A, sizeof(unsigned long int),size, out);
    fclose(in);
    fclose(out);
    return 0;
}
