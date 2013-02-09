all: sequential_mergesort_test parallel_mergesort_test

parallel_mergesort_test: sequential_mergesort.cpp parallel_mergesort.cpp parallel_mergesort.cpp sequential_mergesort.h Makefile
	g++ -O6 -o parallel_mergesort_test sequential_mergesort.cpp parallel_mergesort.cpp -pthread


sequential_mergesort_test: sequential_mergesort.cpp sequential_mergesort_test.cpp sequential_mergesort.h Makefile
	g++ -O6 -o sequential_mergesort_test sequential_mergesort_test.cpp sequential_mergesort.cpp 