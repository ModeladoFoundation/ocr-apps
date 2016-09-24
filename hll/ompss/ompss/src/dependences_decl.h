
#ifndef DEPENDENCE_DECL_H
#define DEPENDENCE_DECL_H

#include <ocr.h>

/*! \brief Specifies what action should be taken when handling
 *  a dependency event _after_ EDT's execution*/
enum _dep_post_action {
    SATISFY = 1, /*!< Satisfy second precondition slot */
    DESTROY = 2  /*!< Destroy event */
};

/*! \brief Specifies what action should be taken when handling
 *  a dependency event _before_ EDT's execution*/
enum _dep_pre_action {
    DEPEND_ONLY = 1,      /*!< Add a dependency to this event */
    DEPEND_AND_SATISFY = 2/*!< Add a dependency and satisfy
                           *   its second precondition slot */
};

struct _acquire_dep {
    ocrGuid_t            event;  /*!< Event that represents an edge of
                                  *  the task dependency graph */
    enum _dep_pre_action action; /*!< Event operation */
};

struct _release_dep {
    ocrGuid_t             event;  /*!< Event that represents an edge of
                                   *   the task dependency graph */
    enum _dep_post_action action; /*!< Event operation */
};

typedef struct {
    /*! Latch event that triggers when all read-only accesses are completed
     *  AND the read-only section is over (a write access
     *  has been registered). */
    ocrGuid_t rd_is_done_evt;
    /*! Sticky event that triggers when a write-only or read-write access is
     *  completed. */
    ocrGuid_t wr_is_done_evt;
} data_dependency_t;

#endif // DEPENDENCE_DECL_H

