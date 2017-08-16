
#
# Runs static Stencil-2D VS skeleton version
#

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP not defined"
    exit 1
fi

cd ${REPO_TOP}/apps/apps/Stencil2D/refactored/ocr/intel-channelEVTs

OCR_VARIANT_LIST="ocr_install_mpiicc_0412-nodebug"

APP_VARIANT_LIST="install_static_skeleton install_static_eager_skeleton"

CMD=$1

JOB_PROPERTIES=${JOB_PROPERTIES-./scripts/job.properties}
. ${JOB_PROPERTIES}

CPGN_BASE=${CPGN}

for OCR_VARIANT in `echo ${OCR_VARIANT_LIST}`; do
	ocrName=${OCR_VARIANT##*/}
	export OCR_INSTALL=${HOME}/${OCR_VARIANT}
	for APP_VARIANT in `echo ${APP_VARIANT_LIST}`; do
		appName=${APP_VARIANT##*/}
		export APP_VARIANT_PATH=${APP_VARIANT}
		CPGN="${CPGN_BASE}-${ocrName}-${appName}" ./scripts/driver.sh $@
	done
done
