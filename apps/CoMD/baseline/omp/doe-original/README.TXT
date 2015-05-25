This is the OpenMP implementation of CoMD version 1.1
=====================================================

To contact the developers of CoMD send email to exmatex-comd@llnl.gov.

CoMD is a reference implementation of typical classical molecular
dynamics algorithms and workloads.  It is created and maintained by
The Exascale Co-Design Center for Materials in Extreme Environments
(ExMatEx).  http://codesign.lanl.gov/projects/exmatex.  The
code is intended to serve as a vehicle for co-design by allowing
others to extend and/or reimplement it as needed to test performance of
new architectures, programming models, etc.

The current version of CoMD is available from:

https://github.com/exmatex/CoMD



Obtaining Documentation
=======================

CoMD documentation is produced by doxygen (www.doxygen.org).  See
../src-mpi/README.TXT for more information.

Information specific to the OpenMP version is in OpenMP.dox (human
readable).  If you run doxygen in src-openmp this information will
automatically be included in the generated html files (html/index.html).


Building and running CoMD
=============
Option - 1:

    The OpenMP versin CoMD is written with portability in mind and should
    compile using practically any compiler that implements the C99 standard
    and OpenMP 3.0 or greater.  You will need to create a Makefile by
    copying the sample provided with the distribution (Makefile.vanilla).

    $ cd src-omp
    $ cp Makefile.vanilla Makefile

    and use the make command to build the code

    $ make

    The sample Makefile will compile the code on many platforms.  See
    comments in Makefile.vanilla for information about specifying the name
    of the C compiler and/or additional compiler switches that might be
    necessary for your platform.

OR

Option - 2:
    # Using the unified run framework
    $ source ../../../../apps_env.bash
    $ cd build_and_run
    $ cd omp
    $ run_app ss_small 10 8 #Runs the "small" problem size defined in ../../../../parameters.proposed file for 10 timesteps using 8 OpenMP threads
