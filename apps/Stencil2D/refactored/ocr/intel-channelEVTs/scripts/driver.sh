
# Job parameters
export c=${c-"2 5 17"}
export n=${n-"1 4 16 64"}
BATCH_TPL=${BATCH_TPL-Stencil2D}

# Application parameters
# The driver script will interpret 'WORKLOAD_ARGS_NAMES' as a list of workload arguments names
# It is expected there will be environment variables with the exact same names defining
# a list of values for that workload argument.
#

# Warning MUST NOT start with space and MUST end with space
WORKLOAD_ARGS_NAMES="iter tile "

iter=${iter-"100 400 800 1600"}
tile=${tile-"360 768 1536 3072"}

#TODO need to do that more cleanly
# Campaign setup
CPGN=${CPGN-"xp-thor-tile-sweep"}

#
# Customizable variables
#
# OCR_VARIANT_PATH
# APP_VARIANT_PATH
# REPO_TOP
# MHOME: filesystem's home root
# CPGN : Output folder
APPS_ROOT="$PWD/../../../.."

. ${APPS_ROOT}/tools/execution_tools/batch/driver.sh
