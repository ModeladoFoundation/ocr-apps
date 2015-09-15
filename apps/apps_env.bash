#------------------------------------------------------------------------------
# env.bash
#
# Setup your environment for the XSTACK apps
#
# this assumes bash is your shell
# this is not a shell script, you have to 'source' it either by:
#
# source apps_env.bash
# -or-
# . apps_env.bash
#------------------------------------------------------------------------------

#PATHS=( tools/local/bin bin sim sim/tools )
PATHS=( tools/execution_tools )

#------------------------------------------------------------------------------
# remove_from_path
# this helper function removes a directory from the path
#------------------------------------------------------------------------------
remove_from_path() {
    echo Warning: Removing path $1
    export PATH=`echo -n $PATH | awk -v RS=: -v ORS=: '$0 != "'$1'"' | sed 's/:$//'`;
}

#------------------------------------------------------------------------------
# add_to_path
# this helper function only adds a directory to the path if it is not already
# in it. This helps reduce duplicates in your path
#------------------------------------------------------------------------------
add_to_path() {
    if [[ ":$PATH:" != *":$1:"* ]]; then
        export PATH="$PATH:$1"
    fi
}

# check if the environment has already been set and if it has, remove any
# any paths based on the current APPS_ROOT so we can set fresh ones
if [ $APPS_ROOT ]; then
    echo Warning: Found existing APPS_ROOT=$APPS_ROOT
    for p in "${PATHS[@]}"
        do
            remove_from_path "$APPS_ROOT/$p"
        done
    echo
fi

# set repository-wide environment variables
export APPS_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo APPS_ROOT=$APPS_ROOT
export XSTACK_ROOT="$( cd "$( dirname "${APPS_ROOT}" )" && pwd )"
echo XSTACK_ROOT=$XSTACK_ROOT

# add paths
for p in "${PATHS[@]}"
    do
        add_to_path "$APPS_ROOT/$p"
    done

#------------------------------------------------------------------------------
# setup icc compiler variables
#------------------------------------------------------------------------------
if [ -x /opt/intel/tools/psxevars.sh ]; then
    source /opt/intel/tools/psxevars.sh intel64 impi4
fi
