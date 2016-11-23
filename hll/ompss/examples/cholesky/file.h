
#ifndef FILE_H
#define FILE_H

void load_values( const char* filename, double** matrix, unsigned* n );

void store_values( const char* filename, double* matrix, unsigned n );

void generate_values( const char* filename, double *matrix, unsigned n );

#endif // FILE_H

