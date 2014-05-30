#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__
struct distribution {
    int dummy; // for swarm2c compile error
};

typedef struct distribution Dist;

Dist Dist_create(int dist_type);
void Dist_init(Dist *d, int dist_type);
int Dist_get_home(const Dist *dist, int i);
void Dist_print(const Dist *d);

#endif
