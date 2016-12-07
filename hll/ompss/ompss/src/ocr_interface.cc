
#include "ompss_ocr_interface.h"
#include "task/task_scope.h"

extern "C" {

void ompss_local_setup( ompss_local_t* handle ) {
    new(static_cast<void*>(handle)) ompss::TaskScopeInfo();
}

void ompss_local_teardown( ompss_local_t* handle ) {
    reinterpret_cast<ompss::TaskScopeInfo*>(handle)->~TaskScopeInfo();
}

}

