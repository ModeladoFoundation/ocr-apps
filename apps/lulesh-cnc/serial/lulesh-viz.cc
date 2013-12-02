#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <silo.h>
#include <math.h>

#include "lulesh.h"


// Function prototypes
static void DumpDomainToVisit(DBfile *db, Domain& domain);
static void DumpMultiblockObjects(DBfile *db, char basename[]);

/**********************************************************************/
void DumpToVisit(Domain& domain, int numFiles)
{
	char subdirName[32];
	char basename[32];
	DBfile *db;

	sprintf(basename, "lulesh_plot_c%d", domain.cycle());
	sprintf(subdirName, "data_0");

	db = (DBfile*)DBCreate(basename, DB_CLOBBER, DB_LOCAL, NULL, DB_HDF5X);

	if (db) {
		DBMkDir(db, subdirName);
		DBSetDir(db, subdirName);
		DumpDomainToVisit(db, domain);
		DumpMultiblockObjects(db, basename);
	}
	else {
		printf("Error writing out viz file\n");
	}
}


static void DumpDomainToVisit(DBfile *db, Domain& domain)
{
	int ok = 0;

	/* Create an option list that will give some hints to VisIt for
	 * printing out the cycle and time in the annotations */
	DBoptlist *optlist;


	/* Write out the mesh connectivity in fully unstructured format */
	int shapetype[1] = {DB_ZONETYPE_HEX};
	int shapesize[1] = {8};
	int shapecnt[1] = {domain.numElem()};
	int *conn = new int[domain.numElem()*8] ;
	int ci = 0 ;
	for (int ei=0; ei < domain.numElem(); ++ei) {
		Index_t *elemToNode = domain.nodelist(ei) ;
		for (int ni=0; ni < 8; ++ni) {
			conn[ci++] = elemToNode[ni] ;
		}
	}
	ok += DBPutZonelist2(db, "connectivity", domain.numElem(), 3,
			conn, domain.numElem()*8,
			0,0,0, /* Not carrying ghost zones */
			shapetype, shapesize, shapecnt,
			1, NULL);
	delete [] conn ;

	/* Write out the mesh coordinates associated with the mesh */
	const char* coordnames[3] = {"X", "Y", "Z"};
	Real_t *coords[3] ;
	coords[0] = new double[domain.numNode()] ;
	coords[1] = new double[domain.numNode()] ;
	coords[2] = new double[domain.numNode()] ;
	for (int ni=0; ni < domain.numNode() ; ++ni) {
		coords[0][ni] = domain.x(ni) ;
		coords[1][ni] = domain.y(ni) ;
		coords[2][ni] = domain.z(ni) ;
	}
	optlist = DBMakeOptlist(2);
	ok += DBAddOption(optlist, DBOPT_DTIME, &domain.time());
	ok += DBAddOption(optlist, DBOPT_CYCLE, &domain.cycle());
	ok += DBPutUcdmesh(db, "mesh", 3, (char**)&coordnames[0], (float**)coords,
			domain.numNode(), domain.numElem(), "connectivity",
			0, DB_DOUBLE, optlist);
	ok += DBFreeOptlist(optlist);
	delete [] coords[2] ;
	delete [] coords[1] ;
	delete [] coords[0] ;

	/* Write out the materials */
	int *matnums = new int[domain.numReg()];
	int dims[1] = {domain.numElem()}; // No mixed elements
	for(int i=0 ; i<domain.numReg() ; ++i)
		matnums[i] = i+1;

	ok += DBPutMaterial(db, "regions", "mesh", domain.numReg(),
			matnums, domain.regNumList(), dims, 1,
			NULL, NULL, NULL, NULL, 0, DB_DOUBLE, NULL);
	delete [] matnums;

	/* Write out pressure, energy, relvol, q */

	Real_t *e = new double[domain.numElem()] ;
	for (int ei=0; ei < domain.numElem(); ++ei) {
		e[ei] = domain.e(ei) ;
	}
	ok += DBPutUcdvar1(db, "e", "mesh", (float*) e,
			domain.numElem(), NULL, 0, DB_DOUBLE, DB_ZONECENT,
			NULL);
	delete [] e ;


	Real_t *p = new double[domain.numElem()] ;
	for (int ei=0; ei < domain.numElem(); ++ei) {
		p[ei] = domain.p(ei) ;
	}
	ok += DBPutUcdvar1(db, "p", "mesh", (float*) p,
			domain.numElem(), NULL, 0, DB_DOUBLE, DB_ZONECENT,
			NULL);
	delete [] p ;

	Real_t *v = new double[domain.numElem()] ;
	for (int ei=0; ei < domain.numElem(); ++ei) {
		v[ei] = domain.v(ei) ;
	}
	ok += DBPutUcdvar1(db, "v", "mesh", (float*) v,
			domain.numElem(), NULL, 0, DB_DOUBLE, DB_ZONECENT,
			NULL);
	delete [] v ;

	Real_t *q = new double[domain.numElem()] ;
	for (int ei=0; ei < domain.numElem(); ++ei) {
		q[ei] = domain.q(ei) ;
	}
	ok += DBPutUcdvar1(db, "q", "mesh", (float*) q,
			domain.numElem(), NULL, 0, DB_DOUBLE, DB_ZONECENT,
			NULL);
	delete [] q ;

	/* Write out nodal speed, velocities */
	Real_t *zd    = new double[domain.numNode()];
	Real_t *yd    = new double[domain.numNode()];
	Real_t *xd    = new double[domain.numNode()];
	Real_t *speed = new double[domain.numNode()];
	for(int ni=0 ; ni < domain.numNode() ; ++ni) {
		xd[ni]    = domain.xd(ni);
		yd[ni]    = domain.yd(ni);
		zd[ni]    = domain.zd(ni);
		speed[ni] = sqrt((xd[ni]*xd[ni])+(yd[ni]*yd[ni])+(zd[ni]*zd[ni]));
	}

	ok += DBPutUcdvar1(db, "speed", "mesh", (float*)speed,
			domain.numNode(), NULL, 0, DB_DOUBLE, DB_NODECENT,
			NULL);
	delete [] speed;


	ok += DBPutUcdvar1(db, "xd", "mesh", (float*) xd,
			domain.numNode(), NULL, 0, DB_DOUBLE, DB_NODECENT,
			NULL);
	delete [] xd ;

	ok += DBPutUcdvar1(db, "yd", "mesh", (float*) yd,
			domain.numNode(), NULL, 0, DB_DOUBLE, DB_NODECENT,
			NULL);
	delete [] yd ;

	ok += DBPutUcdvar1(db, "zd", "mesh", (float*) zd,
			domain.numNode(), NULL, 0, DB_DOUBLE, DB_NODECENT,
			NULL);
	delete [] zd ;


	if (ok != 0) {
		printf("Error writing out viz file\n");
	}
}

void DumpMultiblockObjects(DBfile *db, char basename[])
{
	/* MULTIBLOCK objects to tie together multiple files */
	char **multimeshObjs;
	char **multimatObjs;
	char ***multivarObjs;
	int *blockTypes;
	int *varTypes;
	int ok = 0;
	// Make sure this list matches what's written out above
	char vars[][10] = {"p","e","v","q", "speed", "xd", "yd", "zd"};
	int numvars = sizeof(vars)/sizeof(vars[0]);

	// Reset to the root directory of the silo file
	DBSetDir(db, "/");

	// Allocate a bunch of space for building up the string names
	multimeshObjs = new char*[1];
	multimatObjs = new char*[1];
	multivarObjs = new char**[numvars];
	blockTypes = new int[1];
	varTypes = new int[1];

	for(int v=0 ; v<numvars ; ++v) {
		multivarObjs[v] = new char*[1];
	}

	for(int i=0 ; i< 1; ++i) {
		multimeshObjs[i] = new char[64];
		multimatObjs[i] = new char[64];
		for(int v=0 ; v<numvars ; ++v) {
			multivarObjs[v][i] = new char[64];
		}
		blockTypes[i] = DB_UCDMESH;
		varTypes[i] = DB_UCDVAR;
	}

	// Build up the multiobject names
	for(int i=0 ; i< 1 ; ++i) {
		int iorank = 0;

		//delete multivarObjs[i];
		if (iorank == 0) {
			snprintf(multimeshObjs[i], 64, "/data_%d/mesh", i);
			snprintf(multimatObjs[i], 64, "/data_%d/regions",i);
			for(int v=0 ; v<numvars ; ++v) {
				snprintf(multivarObjs[v][i], 64, "/data_%d/%s", i, vars[v]);
			}

		}
		else {
			snprintf(multimeshObjs[i], 64, "%s.%03d:/data_%d/mesh",
					basename, iorank, i);
			snprintf(multimatObjs[i], 64, "%s.%03d:/data_%d/regions",
					basename, iorank, i);
			for(int v=0 ; v<numvars ; ++v) {
				snprintf(multivarObjs[v][i], 64, "%s.%03d:/data_%d/%s",
						basename, iorank, i, vars[v]);
			}
		}
	}

	// Now write out the objects
	ok += DBPutMultimesh(db, "mesh", 1, (char**)multimeshObjs, blockTypes, NULL);
	ok += DBPutMultimat(db, "regions", 1, (char**)multimatObjs, NULL);
	for(int v=0 ; v<numvars ; ++v) {
		ok += DBPutMultivar(db, vars[v], 1, (char**)multivarObjs[v], varTypes, NULL);
	}

	for(int v=0; v < numvars; ++v) {
		for(int i = 0; i < 1; i++) {
			delete multivarObjs[v][i];
		}
		delete multivarObjs[v];
	}

	// Clean up
	for(int i=0 ; i<1 ; i++) {
		delete multimeshObjs[i];
		delete multimatObjs[i];
	}
	delete [] multimeshObjs;
	delete [] multimatObjs;
	delete [] multivarObjs;
	delete [] blockTypes;
	delete [] varTypes;

	if (ok != 0) {
		printf("Error writing out multiXXX objs to viz file\n");
	}
}


