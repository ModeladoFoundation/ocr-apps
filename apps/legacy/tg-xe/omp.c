#include<stdio.h>
#include<omp.h>

int main()
{
    int id;
#pragma omp parallel
{
    id = omp_get_thread_num();
    printf("Greetings from process %d!\n", id);
    printf("nthreads %d\n", omp_get_num_threads());
}
}
