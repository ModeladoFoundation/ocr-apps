
g++ -c rmat_connected_graph.cpp -o rmat_connected_graph.o -I/home/sandeep/Stuff_Flat/libs/include
g++ -o rmat_connected_graph rmat_connected_graph.o -L/home/sandeep/Stuff_Flat/libs/lib -lboost_graph -lboost_regex