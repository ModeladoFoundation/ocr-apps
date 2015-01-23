#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "io_lammps.h"

void allocate(simulation_space *space)
{
    space->atoms = (struct atomType*)calloc(space->number_atoms, sizeof(struct atomType));
}

void read_lammps(char *filename, simulation_space *space)
{
    FILE* handle = fopen(filename, "r");

    if( handle == NULL )
    {
        fprintf(stderr, "Failed opening file %s\n", filename);
        _Exit(1);
    }
    
    char line[250];
    while( !feof(handle) && fgets(line, 250, handle))
    {
        /*E.g. timestep       0.001 */
        if( strncmp(line, "timestep", 8) == 0 )
        {
            char *token = strtok(line, " \t");
            token = strtok(NULL, " \t");
            if( token == NULL )
            {
                fprintf(stderr, "Failed to read timestep from file %s\n", filename);
                _Exit(3);
            }
            space->timestep = strtod(token, NULL); //assign timestep.
        }

        /*E.g. pair_coeff * * 1 1 */
        if( strncmp(line, "pair_coeff", 10) == 0 )
        {
            char *token = strtok(line, " *\t");
            token = strtok(NULL, " *\t");
            if( token == NULL )
            {
                fprintf(stderr, "Failed to read epsilon from file %s\n", filename);
                _Exit(3);
            }
            space->epsilon = strtod(token, NULL);
            token = strtok(NULL, " *\t");
            if( token == NULL )
            {
                fprintf(stderr, "Failed to read sigma from file %s\n", filename);
                _Exit(3);
            }
            space->sigma = strtod(token, NULL);
        }

        /*E.g. pair_style     lj/cut 5.7875 */
        if( strncmp(line, "pair_style", 10) == 0 )
        {
            char *token = strtok(line, " \t");
            token = strtok(NULL, " \t"); //skip.
            token = strtok(NULL, " \t");
            if( token == NULL )
            {
                fprintf(stderr, "Failed to read cutoff from file %s\n", filename);
                _Exit(3);
            }
            space->cutoff = strtod(token, NULL);
        }

        /*E.g. run 1000 */
        if( strncmp(line, "run", 3) == 0 )
        {
            char *token = strtok(line, " \t");
            token = strtok(NULL, " \t");
            if( token == NULL )
            {
                fprintf(stderr, "Failed to read the number of timesteps from file %s\n", filename);
                _Exit(3);
            }
            space->num_timesteps = strtol(token, NULL, 10);
        }
        
        /*E.g. dump x all custom 10 realunits.dump id mass x y z vx vy vz fx fy fz */
        if( strncmp(line, "dump", 4) == 0 )
        {
            char *token = strtok(line, " \t");
            token = strtok(NULL, " \t"); //skip.
            token = strtok(NULL, " \t"); //skip.
            token = strtok(NULL, " \t"); //skip.
            token = strtok(NULL, " \t");
            if( token == NULL )
            {
                fprintf(stderr, "Failed to read the write freq from file %s\n", filename);
                _Exit(3);
            }
            space->write_freq = strtol(token, NULL, 10);
        }
    }
}

void read_file(char *filename, simulation_space *space)
{
    FILE* handle = fopen(filename, "r");

    if( handle == NULL )
    {
        fprintf(stderr, "Failed opening file %s\n", filename);
        _Exit(1);
    }

    typedef enum lammps_attribute {
        LAMMPS_FIELD_UNKNOWN = 0, LAMMPS_FIELD_ATOMID = 1, LAMMPS_FIELD_MOLID, LAMMPS_FIELD_TYPE,
        LAMMPS_FIELD_POSX = 3, LAMMPS_FIELD_POSY = 4, LAMMPS_FIELD_POSZ = 5, LAMMPS_FIELD_POSXS,
        LAMMPS_FIELD_POSYS, LAMMPS_FIELD_POSZS, LAMMPS_FIELD_POSXU, LAMMPS_FIELD_POSYU,
        LAMMPS_FIELD_POSZU, LAMMPS_FIELD_POSXSU, LAMMPS_FIELD_POSYSU, LAMMPS_FIELD_POSZSU,
        LAMMPS_FIELD_IMGX, LAMMPS_FIELD_IMGY, LAMMPS_FIELD_IMGZ, LAMMPS_FIELD_VELX = 6,
        LAMMPS_FIELD_VELY = 7, LAMMPS_FIELD_VELZ = 8, LAMMPS_FIELD_FORX, LAMMPS_FIELD_FORY,
        LAMMPS_FIELD_FORZ, LAMMPS_FIELD_CHARGE, LAMMPS_FIELD_RADIUS, LAMMPS_FIELD_DIAMETER,
        LAMMPS_FIELD_ELEMENT, LAMMPS_FIELD_MASS = 2, LAMMPS_FIELD_QUATW, LAMMPS_FIELD_QUATI,
        LAMMPS_FIELD_QUATJ, LAMMPS_FIELD_QUATK, LAMMPS_FIELD_MUX, LAMMPS_FIELD_MUY,
        LAMMPS_FIELD_MUZ, LAMMPS_FIELD_USER0, LAMMPS_FIELD_USER1, LAMMPS_FIELD_USER2,
        LAMMPS_FIELD_USER3, LAMMPS_FIELD_USER4, LAMMPS_FIELD_USER5, LAMMPS_FIELD_USER6,
        LAMMPS_FIELD_USER7, LAMMPS_FIELD_USER8, LAMMPS_FILED_USER9
    } l_attr_t;
    typedef enum { FIXED = 0, PERIODIC = 1 } periodicity ;
    
    char line[250];
    int field_map[50] = {0};
    int number_fields = 0, current_atom = 0;
    periodicity per[3] = {FIXED};

    while( !feof(handle) && fgets(line, 250, handle))
        if( strstr(line, "ITEM: TIMESTEP") )
        {
            fgets(line, 250, handle);
            int r = sscanf(line, "%ld", &space->initial_iteration);
            if( r != 1 )
            {
                fprintf(stderr, "Failed to read timestep from file %s\n", filename);
                _Exit(3);
            }
        }

        else if( strstr(line, "ITEM: NUMBER OF ATOMS") )
        {
            fgets(line, 250, handle);
            int r = sscanf(line, "%ld", &(space->number_atoms));
            if( r != 1 || space->number_atoms <= 0 )
            {
                fprintf(stderr, "Failed to read number of atoms from file %s\n", filename);
                _Exit(4);
            }
        }

        else if( strstr(line, "ITEM: BOX BOUNDS") )
        {
            char *k = strtok(line," \t\n\r"); // ITEM
            strtok(NULL," \t\n\r"); // BOX
            strtok(NULL," \t\n\r"); // BOUNDS
            int i = 0;
            
            while( (k = strtok(NULL," \t\n\r")) != NULL )
            {
                if(0 == strcmp(k, "ff"))
                    per[i] = FIXED;
                else if(0 == strcmp(k, "pp"))
                    per[i] = PERIODIC;
                else
                {
                    fprintf(stderr, "Unexpected periodicity type in file %s : \n%s\n", filename, k);
                    _Exit(5);
                }

                if( ++i >= 3 )
                    break;
            }

            space->period_x = per[0];
            space->period_y = per[1];
            space->period_z = per[2];

            char *next;
            fgets(line, 250, handle);
            space->lo.x = strtod(line, &next);
            space->hi.x = strtod(next, NULL);
            fgets(line, 250, handle);
            space->lo.y = strtod(line, &next);
            space->hi.y = strtod(next, NULL);
            fgets(line, 250, handle);
            space->lo.z = strtod(line, &next);
            space->hi.z = strtod(next, NULL);
        }

        else if( strstr(line, "ITEM: ATOMS") )
        {
            // end of this line says which data we will know about the atoms in the next section
            char *k = strtok(line," \t\n\r"); // ITEM
            strtok(NULL," \t\n\r"); // ATOMS
            int i = -1;
            l_attr_t data_type;
                   
            while( (k = strtok(NULL," \t\n\r")) != NULL )
            {
                ++i;

                if (0 == strcmp(k, "id")) {
                    data_type = LAMMPS_FIELD_ATOMID;
//                } else if (0 == strcmp(k, "mol")) {
//                    data_type = LAMMPS_FIELD_MOLID;
//                } else if (0 == strcmp(k, "type")) {
//                    data_type = LAMMPS_FIELD_TYPE;
                } else if (0 == strcmp(k, "x")) {
                    data_type = LAMMPS_FIELD_POSX;
                } else if (0 == strcmp(k, "y")) {
                    data_type = LAMMPS_FIELD_POSY;
                } else if (0 == strcmp(k, "z")) {
                    data_type = LAMMPS_FIELD_POSZ;
//                } else if (0 == strcmp(k, "xs")) {
//                    data_type = LAMMPS_FIELD_POSXS;
//                } else if (0 == strcmp(k, "ys")) {
//                    data_type = LAMMPS_FIELD_POSYS;
//                } else if (0 == strcmp(k, "zs")) {
//                    data_type = LAMMPS_FIELD_POSZS;
//                } else if (0 == strcmp(k, "xu")) {
//                    data_type = LAMMPS_FIELD_POSXU;
//                } else if (0 == strcmp(k, "yu")) {
//                    data_type = LAMMPS_FIELD_POSYU;
//                } else if (0 == strcmp(k, "zu")) {
//                    data_type = LAMMPS_FIELD_POSZU;
//                } else if (0 == strcmp(k, "xus")) {
//                    data_type = LAMMPS_FIELD_POSXU;
//                } else if (0 == strcmp(k, "yus")) {
//                    data_type = LAMMPS_FIELD_POSYU;
//                } else if (0 == strcmp(k, "zus")) {
//                    data_type = LAMMPS_FIELD_POSZU;
//                } else if (0 == strcmp(k, "ix")) {
//                    data_type = LAMMPS_FIELD_IMGX;
//                } else if (0 == strcmp(k, "iy")) {
//                    data_type = LAMMPS_FIELD_IMGY;
//                } else if (0 == strcmp(k, "iz")) {
//                    data_type = LAMMPS_FIELD_IMGZ;
                } else if (0 == strcmp(k, "vx")) {
                    data_type = LAMMPS_FIELD_VELX;
                } else if (0 == strcmp(k, "vy")) {
                    data_type = LAMMPS_FIELD_VELY;
                } else if (0 == strcmp(k, "vz")) {
                    data_type = LAMMPS_FIELD_VELZ;
//                } else if (0 == strcmp(k, "fx")) {
//                    data_type = LAMMPS_FIELD_FORX;
//                } else if (0 == strcmp(k, "fy")) {
//                    data_type = LAMMPS_FIELD_FORY;
//                } else if (0 == strcmp(k, "fz")) {
//                    data_type = LAMMPS_FIELD_FORZ;
//                } else if (0 == strcmp(k, "q")) {
//                    data_type = LAMMPS_FIELD_CHARGE;
//                    *optflags |= MOLFILE_CHARGE; 
//                } else if (0 == strcmp(k, "radius")) {
//                    data_type = LAMMPS_FIELD_RADIUS;
//                    *optflags |= MOLFILE_RADIUS; 
//                } else if (0 == strcmp(k, "diameter")) {
//                    data_type = LAMMPS_FIELD_RADIUS;
//                } else if (0 == strcmp(k, "element")) {
//                    data_type = LAMMPS_FIELD_ELEMENT;
                } else if (0 == strcmp(k, "mass")) {
                    data_type = LAMMPS_FIELD_MASS;
//                } else if (0 == strcmp(k, "mux")) {
//                    data_type = LAMMPS_FIELD_MUX;
//                } else if (0 == strcmp(k, "muy")) {
//                    data_type = LAMMPS_FIELD_MUY;
//                } else if (0 == strcmp(k, "muz")) {
//                    data_type = LAMMPS_FIELD_MUZ;
                } else {
                    // silently ignore
                    continue;
                }

                number_fields++;
                field_map[ i ] = data_type;
            }

            // remember to allocate space for data before going into reading it
            allocate(space);
        }
        else if( number_fields == 0 )
        {
            // not data because we haven't had the list of fields yet...
            fprintf(stderr, "Impossible to read line from file %s : \n%s\n", filename, line);
            _Exit(2);
        }
        else
        {
            // defaulting to data
            char *k = strtok(line, " \t");
            double fields[10];
            int i = 0, c = 0;

            do {
                if( field_map[i] )
                {
                    fields[ field_map[i] ] = strtod(k,NULL);
                    c++;
                }
                i++;
            }
            while( (k = strtok(NULL," \t")) != NULL && c < number_fields);

            if( c < number_fields )
            {
                // not data because we haven't had the list of fields yet...
                fprintf(stderr, "Impossible to read some of the data from file %s\n", filename);
                _Exit(7);
            }

            space->atoms[current_atom].id        = (uint64_t)fields[ LAMMPS_FIELD_ATOMID ];
            space->atoms[current_atom].mass        = fields[ LAMMPS_FIELD_MASS ];
            space->atoms[current_atom].pos_x    = fields[ LAMMPS_FIELD_POSX ];
            space->atoms[current_atom].pos_y    = fields[ LAMMPS_FIELD_POSY ];
            space->atoms[current_atom].pos_z    = fields[ LAMMPS_FIELD_POSZ ];
            space->atoms[current_atom].mom_x    = fields[ LAMMPS_FIELD_VELX ]*space->atoms[current_atom].mass;
            space->atoms[current_atom].mom_y    = fields[ LAMMPS_FIELD_VELY ]*space->atoms[current_atom].mass;
            space->atoms[current_atom].mom_z    = fields[ LAMMPS_FIELD_VELZ ]*space->atoms[current_atom].mass;

            
            current_atom++;
        }

    fclose(handle);
}

///FIXME: momentum is being printed -- WRONG.
void write_out(FILE* handle, simulation_space *space, int64_t time, double kinetic, double potential)
{
    fprintf(handle, "ITEM: TIMESTEP\n%ld\nITEM: NUMBER OF ATOMS\n%ld\nITEM: BOX BOUNDS %s %s %s\n%g %g\n%g %g\n%g %g\nITEM: ATOMS id mass x y z vx vy vz fx fy fz\n",
        time, space->number_atoms, space->period_x?"pp":"ff", space->period_y?"pp":"ff", space->period_z?"pp":"ff",
        space->lo.x, space->hi.x, space->lo.y, space->hi.y, space->lo.z, space->hi.z);
    
    int i;
    for(i=0; i<space->number_atoms; i++)    
        fprintf(handle, "%ld %g %g %g %g %g %g %g %g %g %g\n", space->atoms[i].id, space->atoms[i].mass, 
            space->atoms[i].pos_x, space->atoms[i].pos_y, space->atoms[i].pos_z,
            space->atoms[i].mom_x/space->atoms[i].mass, space->atoms[i].mom_y/space->atoms[i].mass, space->atoms[i].mom_z/space->atoms[i].mass,
            space->atoms[i].for_x, space->atoms[i].for_y, space->atoms[i].for_z);

    fprintf(handle, "ITEM: POTENTIAL ENERGY\n%.15e\nITEM: KINETIC ENERGY\n%.15e\n", potential, kinetic);
}

void write_file(char *filename, simulation_space *space, int64_t time, double kinetic, double potential)
{
    FILE* handle = fopen(filename, "w");
    write_out(handle, space, time, kinetic, potential);
    fclose(handle);
}

