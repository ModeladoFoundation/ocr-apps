This directory contains the implementation of the molecular dynamics simulation using Intel Concurrent Collections
The directory structure is as follows:

root:
-- md.cpp
-- md.h
-- steps/
   -- force.h
   -- force.hpp
   -- redistribute.h
   -- redistribute.h
   -- velocity_position.h
   -- velocity_position.hpp
-- utils/
   -- types.h
   -- io.h
   -- celltypes.h
   -- particletypes.h
   -- forcetypes.h
-- tuners/
   -- itemtuners.h
   -- itemtuners.hpp
   -- steptuners.h
   -- steptuners.hpp
   -- policy*.h

Setting up the CNC environment:
-------------------------------
CnC needs the Intel Thread Building Blocks library to function.
Ensure that the environment variables $CNCROOT, $TBBROOT are set correctly
Source the cncvars.sh file in the <CNCROOT> folder as follows (for bash environment)
cd <CNCROOT>
. env.sh

Compiling the code:
--------------------
Use the accompanying Makefile to compile the code. Change the Makefile as needed
In case of distributed CnC, specify the DIST_CNC variable accordingly e.g. DIST_CNC=MPI
Two final executables are generated: "md" and "distmd" for single and distributed setup respectively

Executing the code:
--------------------
usage: md/distmd <input-file> <output-dir> <epsilon> <sigma> <cutoff> <shift-pot> <num-iterations> <print-interval> <timestep> <numcells-x> <numcells-y> <numcells-z>

For convenience a script runmd.sh has been included. Please see ./runmd.sh -h for usage details

The output files are generated in the directory <output-dir> with the extension .out
For distributed mode, ensure the env variable $LD_LIBRARY_PATH is set correctly

Sample input files are provided in the folder inputfiles
Reference output files are located in the directory refout

Tuning
------
The file policy.h in the tuners/ directory is used to define the mapping of cell ids to processors.
Change this file to change data distribution
