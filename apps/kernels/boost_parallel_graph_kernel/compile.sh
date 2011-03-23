
g++ -c rmat_gen.cpp -o rmat_gen.o -I/home/sandeep/Stuff_Flat/libs/include -I/usr/include/mpi

g++ -o rmat_gen rmat_gen.o -L/home/sandeep/Stuff_Flat/libs/lib -lboost_graph -lboost_regex -lboost_graph_parallel -lboost_serialization -lboost_system


g++ -c bfs_benchmark.cpp -o bfs_benchmark.o -I/home/sandeep/Stuff_Flat/libs/include -I/usr/include/mpi

g++ -o bfs_benchmark bfs_benchmark.o -L/home/sandeep/Stuff_Flat/libs/lib -lboost_graph -lboost_regex -lboost_graph_parallel -lboost_serialization -lboost_system