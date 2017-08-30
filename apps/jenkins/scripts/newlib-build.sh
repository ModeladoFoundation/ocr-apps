#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage <sub-directory> <arch>"
    exit 1
fi

# If APPS_MAKEFILE is not set, use WORKLOAD_SRC/Makefile
APPS_MAKEFILE=${APPS_MAKEFILE:-${WORKLOAD_SRC}/Makefile}

if [ "${WITH_NEWLIB}" == "yes" ]; then
    echo "Building '$1' for architecture '$2'"

    # Must be in newlib folder when invoking the makefile
    # as it relies on CURDIR.

    cd ${APPS_ROOT_PRIV}/${T_PATH}
    make -j1 ARCH=$2 tg-xe-static
    RETURN_CODE=$?

    if [ $RETURN_CODE -eq 0 ]; then
	    echo "**** Build SUCCESS ****"
	    else
	    echo "**** Build FAILURE ****"
	    fi
else
    echo "newlib build not requested (WITH_NEWLIB not set or not yes)"
    RETURN_CODE=0
fi

exit $RETURN_CODE
