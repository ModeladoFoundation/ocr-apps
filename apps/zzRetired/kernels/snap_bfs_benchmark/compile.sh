
gcc -fopenmp -I/home/sandeep/Stuff_Flat/libs/include -L/home/sandeep/Stuff_Flat/libs/lib -o rmat-gen.o -c rmat-gen.c
/bin/bash libtool --tag=CC   --mode=link gcc -std=gnu99  -O3 -fomit-frame-pointer -malign-double -fstrict-aliasing -ffast-math -march=prescott -fopenmp -L/home/Stuff_Flat/libs/lib -lm  -o rmat-gen rmat-gen.o /home/sandeep/Stuff_Flat/libs/lib/libsnap.la -lm



gcc -O3 -fopenmp -I/home/sandeep/Stuff_Flat/libs/include -L/home/sandeep/Stuff_Flat/libs/lib rmat-bfs.c -c rmat-bfs.o
/bin/bash libtool --tag=CC   --mode=link gcc -std=gnu99  -O3 -fomit-frame-pointer -malign-double -fstrict-aliasing -ffast-math -march=prescott -fopenmp -L/home/sandeep/Stuff_Flat/libs/lib -lm  -o rmat-bfs rmat-bfs.o /home/sandeep/Stuff_Flat/libs/lib/libsnap.la -lm


