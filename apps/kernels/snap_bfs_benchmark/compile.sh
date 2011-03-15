
gcc -fopenmp -I../libs/include -L../libs/lib -o rmat-gen.o -c rmat-gen.c
/bin/bash libtool --tag=CC   --mode=link gcc -std=gnu99  -O3 -fomit-frame-pointer -malign-double -fstrict-aliasing -ffast-math -march=prescott -fopenmp -L../libs/lib -lm  -o rmat-gen rmat-gen.o ../libs/lib/libsnap.la -lm



gcc -fopenmp -I../libs/include -L../libs/lib rmat-bfs.c -c rmat-bfs.o 
/bin/bash libtool --tag=CC   --mode=link gcc -std=gnu99  -O3 -fomit-frame-pointer -malign-double -fstrict-aliasing -ffast-math -march=prescott -fopenmp -L../libs/lib -lm  -o rmat-bfs rmat-bfs.o ../libs/lib/libsnap.la -lm


