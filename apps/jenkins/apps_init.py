job_apps_init = {
    'name': 'apps-init-job',
    'jobtype': 'apps-init',
    'sandbox': ('inherit0', ),
    'run-args': ''
}

job_apps_tg_init = {
    'name': 'apps-tg-init-job',
    'jobtype': 'apps-tg-init',
    'sandbox': ('inherit0', ),
    'run-args': ''
}

job_newlib_build = {
    'name': 'newlib-build-job',
    'jobtype': 'newlib-build',
    'run-args': 'newlib tg-xe',
    'sandbox': ('inherit0', )
}
