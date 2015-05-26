CoMD
====

Classical molecular dynamics proxy application.

This is CoMD version 1.1
------------------------

CoMD is a reference implementation of typical classical molecular
dynamics algorithms and workloads.  It is created and maintained by
ExMatEx: Exascale Co-Design Center for Materials in Extreme Environments
(<a href="http://exmatex.org">exmatex.org</a>).  The
code is intended to serve as a vehicle for co-design by allowing
others to extend and/or reimplement it as needed to test performance of
new architectures, programming models, etc.

To view the generated Doxygen documentation for CoMD, please visit
<a href="http://exmatex.github.io/CoMD/doxygen-mpi/index.html">exmatex.github.io/CoMD/doxygen-mpi/index.html</a>.

To contact the developers of CoMD send email to exmatex-comd@llnl.gov.

Building and running CoMD
=============
Option - 1:
    $ cd src-mpi
    $ cp Makefile.vanilla Makefile

    and use the make command to build the code

    $ make

    The sample Makefile will compile the code on many platforms.  See
    comments in Makefile.vanilla for information about specifying the name
    of the C compiler and/or additional compiler switches that might be
    necessary for your platform.

    The executable is created in bin/
    $ ./CoMD-mpi -h #for help running the app

OR

Option - 2:
    # Using the unified run framework
    $ source ../../../../apps_env.bash
    $ cd build_and_run
    $ cd serial #cd mpi #for MPI
    $ run_app ss_small 10 #Runs the "small" problem size defined in ../../../parameters.proposed file for 10 timesteps
