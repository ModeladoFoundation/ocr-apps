
#
# Run OCR embedded profiler over a specific Stencil-2D configuration
#

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP not defined"
    exit 1
fi

cd ${REPO_TOP}/apps/apps/Stencil2D/refactored/ocr/intel-channelEVTs

OCR_VARIANT_LIST=${OCR_VARIANT_LIST-"ocr_install_mpiicc_0414_nodebug_profiled"}

APP_VARIANT_LIST=${APP_VARIANT_LIST-"install_static_eager_skeleton_profiled"}

CMD=$1

export BATCH_TPL=${BATCH_TPL-"Stencil2D-PSL-sglnode"}
export TPLARG_QUEUE=${TPLARG_QUEUE-XAS}
export CPGN=${CPGN-"xp-PSL-test-core-scaling"}
export c=${c-"1 4 16"}
export n=${n-"1"}
export iter=${iter-"3000"}
export tile=${tile-"768"}

JOB_PROPERTIES=${JOB_PROPERTIES-./scripts/job.properties}
. ${JOB_PROPERTIES}

CPGN_BASE=${CPGN}
export HACK_SPECIAL_WORKER="HC"
export TPLARG_OCR_PROFILER=${TPLARG_OCR_PROFILER-yes}
export TPLARG_NB_RUN=${TPLARG_NB_RUN-1}

for OCR_VARIANT in `echo ${OCR_VARIANT_LIST}`; do
	ocrName=${OCR_VARIANT##*/}
	export OCR_INSTALL=${HOME}/${OCR_VARIANT}
	for APP_VARIANT in `echo ${APP_VARIANT_LIST}`; do
		appName=${APP_VARIANT##*/}
		export APP_VARIANT_PATH=${APP_VARIANT}
		echo "APP_VARIANT_PATH=${APP_VARIANT}"
		c_save=${c}
		for coreCount in `echo ${c}`; do
			export c="${coreCount}"
			CPGN="${CPGN_BASE}/${ocrName}-${appName}-${coreCount}c" ./scripts/driver.sh $@
		done
		c=${c_save}
	done
done
