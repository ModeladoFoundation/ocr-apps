
#
# Experiment with NUMA on Stencil-2D
#

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP not defined"
    exit 1
fi

cd ${REPO_TOP}/apps/apps/Stencil2D/refactored/ocr/intel-channelEVTs

# Runs both static and work-stealing scheduler up to 64 Nodes varying input tile size.
# The non-NUMA variant runs a single OCR process per cluster node, while NUMA aware spawns
# one OCR process per socket, per node.

OCR_VARIANT_LIST=${OCR_VARIANT_LIST-"ocr_install_mpiicc_0412-nodebug-affdebug"}

#
APP_VARIANT_LIST=${APP_VARIANT_LIST-"install_static install_static_eager"}

#
NUMA_VARIANT_LIST=${NUMA_VARIANT_LIST-"1 2"}

CMD=$1

JOB_PROPERTIES=${JOB_PROPERTIES-./scripts/job.properties}
. ${JOB_PROPERTIES}

CPGN_BASE=${CPGN}

export TPLARG_OCR_PROFILER=${TPLARG_OCR_PROFILER-no}
export TPLARG_NB_RUN=${TPLARG_NB_RUN-3}

# Launcher-specific
export TPLARG_NUMA_DISTRIB=${TPLARG_NUMA_DISTRIB-"BLOCK"}
# Machine-specific
export TPLARG_NUMA_PACKAGE=${TPLARG_NUMA_PACKAGE-2}
export TPLARG_NUMA_CORES=${TPLARG_NUMA_CORES-18}

# Given a NUMA setting truncate a list of core count to
# those configuration that do not oversubscribe the system
function adaptCoreCountToNUMA() {
	local coreList="$1"
	local newCoreList=""
	if [[ ${NUMA_VARIANT} -eq ${TPLARG_NUMA_PACKAGE} ]]; then
		# Do not oversubscribe sockets
		let maxCount=${TPLARG_NUMA_CORES}
	else # if 1 or different from declared numa package give what the user wants
		let maxCount=$(( ${TPLARG_NUMA_PACKAGE} * ${TPLARG_NUMA_CORES} ))
	fi
	for core in `echo $coreList`; do
		compCore=$(( ${core} - 1 ))
		newCompCore=$(( ${compCore} / ${NUMA_VARIANT} ))
		newCore=$(( ${newCompCore} + 1 ))
		# Need at least two cores for comm+comp
		if [[ ${newCore} -ge 2 ]]; then
			newCoreList="${newCoreList}${newCore} "
		fi
	done
	echo "$newCoreList" | sed -e "s/ $//g"
}

# NOTE: This is handling ocr and apps variant but that could be delegated
# to the execution_tools' driver the tradeoff is that we have more
# control over the campaign folder name formatting
for NUMA_VARIANT in `echo ${NUMA_VARIANT_LIST}`; do
	numaName="numa${NUMA_VARIANT}"
	if [[ ${NUMA_VARIANT} -gt 1 ]]; then # Activates NUMA options for CFG file generation
		export TPLARG_WITH_NUMA="yes"
	else
		export TPLARG_WITH_NUMA="no"
	fi
	echo "NUMA_VARIANT=${NUMA_VARIANT} TPLARG_WITH_NUMA=${TPLARG_WITH_NUMA}"
	for OCR_VARIANT in `echo ${OCR_VARIANT_LIST}`; do
		ocrName=${OCR_VARIANT##*/}
		export OCR_INSTALL=${HOME}/${OCR_VARIANT}
		for APP_VARIANT in `echo ${APP_VARIANT_LIST}`; do
			appName=${APP_VARIANT##*/}
			export APP_VARIANT_PATH=${APP_VARIANT}
			export saveC="${c}"
			export c=`adaptCoreCountToNUMA "${c}"`
			echo "adaptCoreCountToNUMA ${saveC} => ${c}"
			export saveP="${TPLARG_NUMA_PACKAGE}"
			export TPLARG_NUMA_PACKAGE=${NUMA_VARIANT}
			NUMA_VARIANT=${NUMA_VARIANT} CPGN="${CPGN_BASE}-${ocrName}-${appName}-${numaName}" ./scripts/driver.sh $@
			export c="${saveC}"
			export TPLARG_NUMA_PACKAGE="${saveP}"
		done
	done
done
