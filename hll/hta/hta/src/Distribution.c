#include <stdio.h>
#include <stdlib.h>
#include "Distribution.h"

//Dist Dist_create(int dist_type) {
//    Dist ret = {0};
//    return ret;
//}

void Dist_init(Dist *d, int dist_type) {
    d->dummy = 0;
}

int Dist_get_home(const Dist *dist, int i) {
    // FIXME: need implementation
    return -1;
}

void Dist_print(const Dist *d) {
    printf("Dist: unknown\n");
}

