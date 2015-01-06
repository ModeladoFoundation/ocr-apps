// This is a watered down version of the LAMMPS code to parse input files
void read_file(char *filename, md_context& domain)
{
	FILE* handle = fopen(filename, "r");

	if( handle == NULL )
	{
		fprintf(stderr, "Failed opening file %s\n", filename);
		_Exit(1);
	}

	typedef enum lammps_attribute {
		LAMMPS_FIELD_UNKNOWN = 0, LAMMPS_FIELD_ATOMID = 1, LAMMPS_FIELD_MOLID, LAMMPS_FIELD_TYPE = 2,
		LAMMPS_FIELD_POSX = 3, LAMMPS_FIELD_POSY = 4, LAMMPS_FIELD_POSZ = 5, LAMMPS_FIELD_POSXS,
		LAMMPS_FIELD_POSYS, LAMMPS_FIELD_POSZS, LAMMPS_FIELD_POSXU, LAMMPS_FIELD_POSYU,
		LAMMPS_FIELD_POSZU, LAMMPS_FIELD_POSXSU, LAMMPS_FIELD_POSYSU, LAMMPS_FIELD_POSZSU,
		LAMMPS_FIELD_IMGX, LAMMPS_FIELD_IMGY, LAMMPS_FIELD_IMGZ, LAMMPS_FIELD_VELX = 6,
		LAMMPS_FIELD_VELY = 7, LAMMPS_FIELD_VELZ = 8, LAMMPS_FIELD_FORX, LAMMPS_FIELD_FORY,
		LAMMPS_FIELD_FORZ, LAMMPS_FIELD_CHARGE, LAMMPS_FIELD_RADIUS, LAMMPS_FIELD_DIAMETER,
		LAMMPS_FIELD_ELEMENT, LAMMPS_FIELD_MASS, LAMMPS_FIELD_QUATW, LAMMPS_FIELD_QUATI,
		LAMMPS_FIELD_QUATJ, LAMMPS_FIELD_QUATK, LAMMPS_FIELD_MUX, LAMMPS_FIELD_MUY,
		LAMMPS_FIELD_MUZ, LAMMPS_FIELD_USER0, LAMMPS_FIELD_USER1, LAMMPS_FIELD_USER2,
		LAMMPS_FIELD_USER3, LAMMPS_FIELD_USER4, LAMMPS_FIELD_USER5, LAMMPS_FIELD_USER6,
		LAMMPS_FIELD_USER7, LAMMPS_FIELD_USER8, LAMMPS_FILED_USER9
	} l_attr_t;

	char line[250];
	int field_map[50] = {0};
	int number_fields = 0;
	periodicity per[3] = {FIXED};

	md_particle_list_t* particle_list;

	while( !feof(handle) && fgets(line, 250, handle)) {
		if( strstr(line, "ITEM: TIMESTEP") )
		{
			fgets(line, 250, handle);
			int time, r = sscanf(line, "%d", &time);
			if( r != 1 )
			{
				fprintf(stderr, "Failed to read timestep from file %s\n", filename);
				_Exit(3);
			}
			if( time != 0 )
				fprintf(stderr, "I read the timestep should be %d but I'm going to ignore that and start from 0.\n", time);
		}

		else if( strstr(line, "ITEM: NUMBER OF ATOMS") )
		{
			fgets(line, 250, handle);
			int time, r = sscanf(line, "%d", &(domain.val_md_num_particles));
			if( r != 1 || domain.val_md_num_particles <= 0 )
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

			domain.val_md_period_x = per[0];
			domain.val_md_period_y = per[1];
			domain.val_md_period_z = per[2];

			Real_t lo_x, hi_x, lo_y, hi_y, lo_z, hi_z;
			char *next;
			fgets(line, 250, handle);
			lo_x = strtod(line, &next);
			hi_x = strtod(next, NULL);
			fgets(line, 250, handle);
			lo_y = strtod(line, &next);
			hi_y = strtod(next, NULL);
			fgets(line, 250, handle);
			lo_z = strtod(line, &next);
			hi_z = strtod(next, NULL);

			domain.val_md_simbox_lo_x = lo_x;
			domain.val_md_simbox_hi_x = hi_x;
			domain.val_md_simbox_lo_y = lo_y;
			domain.val_md_simbox_hi_y = hi_y;
			domain.val_md_simbox_lo_z = lo_z;
			domain.val_md_simbox_hi_z = hi_z;

			Index_t md_simbox_cx = domain.val_md_simbox_cx;
			Index_t md_simbox_cy = domain.val_md_simbox_cy;
			Index_t md_simbox_cz = domain.val_md_simbox_cz;
			Real_t md_cutoff = domain.val_md_cutoff;

			domain.val_md_cellsize_x = (hi_x - lo_x)/(double)md_simbox_cx;
			domain.val_md_cellsize_y = (hi_y - lo_y)/(double)md_simbox_cy;
			domain.val_md_cellsize_z = (hi_z - lo_z)/(double)md_simbox_cz;

			bool x_valid = ((domain.val_md_period_x == FIXED) || (md_simbox_cx > 2)) ? domain.val_md_cellsize_x >= md_cutoff : domain.val_md_cellsize_x >= 2*md_cutoff;
			bool y_valid = ((domain.val_md_period_y == FIXED) || (md_simbox_cy > 2)) ? domain.val_md_cellsize_y >= md_cutoff : domain.val_md_cellsize_y >= 2*md_cutoff;
			bool z_valid = ((domain.val_md_period_z == FIXED) || (md_simbox_cz > 2)) ? domain.val_md_cellsize_z >= md_cutoff : domain.val_md_cellsize_z >= 2*md_cutoff;

			assert(x_valid && y_valid && z_valid);

			Real_t md_cellsize_x = domain.val_md_cellsize_x;
			Real_t md_cellsize_y = domain.val_md_cellsize_y;
			Real_t md_cellsize_z = domain.val_md_cellsize_z;

			for(Index_t k = 0; k < md_simbox_cz; k++) {
				for(Index_t j = 0; j < md_simbox_cy; j++) {
					for(Index_t i = 0; i < md_simbox_cx; i++) {
						md_cell_index_wrapper_t cell_index(i, j, k);
						domain.tags_md_cell_init_grid.put(cell_index.get_index());
					}
				}
			}

			particle_list = (md_particle_list_t*)(calloc(domain.val_md_num_cells, sizeof(md_particle_list_t)));
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
				} else if (0 == strcmp(k, "mass")) {
					data_type = LAMMPS_FIELD_TYPE;
				} else if (0 == strcmp(k, "x")) {
					data_type = LAMMPS_FIELD_POSX;
				} else if (0 == strcmp(k, "y")) {
					data_type = LAMMPS_FIELD_POSY;
				} else if (0 == strcmp(k, "z")) {
					data_type = LAMMPS_FIELD_POSZ;
				} else if (0 == strcmp(k, "vx")) {
					data_type = LAMMPS_FIELD_VELX;
				} else if (0 == strcmp(k, "vy")) {
					data_type = LAMMPS_FIELD_VELY;
				} else if (0 == strcmp(k, "vz")) {
					data_type = LAMMPS_FIELD_VELZ;
				} else {
					// silently ignore
					continue;
				}

				number_fields++;
				field_map[ i ] = data_type;
			}
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

			md_particle_t p;
			p.a.pid		= (Index_t)fields[ LAMMPS_FIELD_ATOMID ];
			p.a.p.x		= fields[ LAMMPS_FIELD_POSX ];
			p.a.p.y		= fields[ LAMMPS_FIELD_POSY ];
			p.a.p.z		= fields[ LAMMPS_FIELD_POSZ ];
			p.mass		= fields[ LAMMPS_FIELD_TYPE ];
			p.v.x		= fields[ LAMMPS_FIELD_VELX ];
			p.v.y		= fields[ LAMMPS_FIELD_VELY ];
			p.v.z		= fields[ LAMMPS_FIELD_VELZ ];

			Index_t cx,cy,cz;

			cx = (Index_t)floor((p.a.p.x - domain.val_md_simbox_lo_x) / domain.val_md_cellsize_x);
			cy = (Index_t)floor((p.a.p.y - domain.val_md_simbox_lo_y) / domain.val_md_cellsize_y);
			cz = (Index_t)floor((p.a.p.z - domain.val_md_simbox_lo_z) / domain.val_md_cellsize_z);

			md_cell_index_wrapper_t cid = md_cell_index_wrapper_t(cx, cy, cz);

			particle_list[cx + cy*domain.val_md_simbox_cx + cz*domain.val_md_simbox_cx*domain.val_md_simbox_cy].push_back(p);
			// fprintf(stderr, "Atom %d [%.2g,%.2g,%.2g] added to box %s\n", p.a.pid, p.a.p.x, p.a.p.y, p.a.p.z, cid.ctoa().c_str());
		}
	}
	for(Index_t k = 0; k < domain.val_md_simbox_cz; k++) {
		for(Index_t j = 0; j < domain.val_md_simbox_cy; j++) {
			for(Index_t i = 0; i < domain.val_md_simbox_cx; i++) {
				md_rank_cell_t rank_cell = std::make_pair(0, std::make_pair(i, std::make_pair(j, k)));
				domain.items_md_cell_init_particlelist.put(rank_cell, particle_list[i + j*domain.val_md_simbox_cx + k*domain.val_md_simbox_cx*domain.val_md_simbox_cy]);
			}
		}
	}

	free(particle_list);

	fclose(handle);
}
