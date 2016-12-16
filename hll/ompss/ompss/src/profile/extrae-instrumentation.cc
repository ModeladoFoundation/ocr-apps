
#include "extrae-instrumentation.h"

#ifdef _ENABLE_EXTRAE
const char profile::extrae::EventTraits<profile::Event::nanos_register_read_depinfo>::description[] =      "nanos_register_read_depinfo";
const char profile::extrae::EventTraits<profile::Event::nanos_register_readwrite_depinfo>::description[] = "nanos_register_readwrite_depinfo";
const char profile::extrae::EventTraits<profile::Event::nanos_create_task>::description[] =                "nanos_create_task";
const char profile::extrae::EventTraits<profile::Event::nanos_submit_task>::description[] =                "nanos_submit_task";
const char profile::extrae::EventTraits<profile::Event::nanos_taskwait>::description[] =                   "nanos_taskwait";
const char profile::extrae::EventTraits<profile::Event::mainEdt>::description[] =                          "mainEdt";
const char profile::extrae::EventTraits<profile::Event::outlineEdt>::description[] =                       "outlineEdt";
const char profile::extrae::EventTraits<profile::Event::shutdownEdt>::description[] =                      "shutdownEdt";
const char profile::extrae::EventTraits<profile::Event::ompss_user_code>::description[] =                  "OmpSs User code";
#endif

