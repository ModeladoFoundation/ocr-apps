BUILD_DIR=build
TRILINOS_DIR=$(realpath ../publicTrilinos)
#
# Get rid of the current build
#
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
#
# Configure kokkos
#
INSTALL_DIR=$(realpath ../install)
CMAKE="cmake"

CMAKE_ARGS="\
    -DCMAKE_BUILD_TYPE:STRING=DEBUG \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -DTPL_ENABLE_Pthread=OFF \
    -DTrilinos_ENABLE_TESTS:BOOL=OFF \
    -DTrilinos_ENABLE_Kokkos=ON \
    -DTrilinos_ENABLE_Fortran=OFF \
    -DTrilinos_ASSERT_MISSING_PACKAGES=OFF \
    -DBUILD_SHARED_LIBS=OFF"

DOCONF_ARGS="-D Trilinos_ENABLE_Kokkos=ON"

echo Running cmake ...
( cd $BUILD_DIR ; $CMAKE $CMAKE_ARGS $TRILINOS_DIR )

if [ $? -eq 0 ] ; then
    rm -rf $INSTALL_DIR
    mkdir -p $INSTALL_DIR
fi
