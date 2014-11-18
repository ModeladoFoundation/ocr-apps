#ifndef READ_LAMMPS_H
#define READ_LAMMPS_H

#include <stdio.h>
#include <stdint.h>

struct vectType
{
    double x;
    double y;
    double z;
};

struct atomType
{
    int64_t id;
    double mass;
    double for_x;
    double for_y;
    double for_z;
    double pos_x;
    double pos_y;
    double pos_z;
    double mom_x;
    double mom_y;
    double mom_z;
    double kinetic;
    double potential;
};

typedef struct { 
    //space and spatial decomposition
    struct vectType lo, hi;
    int period_x, period_y, period_z; //whether periodic along the boundary.
    // containers for atoms and their informations
    int64_t number_atoms; struct atomType* atoms;
    double timestep, sigma, epsilon, cutoff; int64_t num_timesteps, write_freq, initial_iteration;
} simulation_space;

void read_lammps(char *filename, simulation_space *space);
void read_file(char *filename, simulation_space *space);
void write_out(FILE* handle, simulation_space *space, int64_t time, double kinetic, double potential);
void write_file(char *filename, simulation_space *space, int64_t time, double kinetic, double potential);

#endif

