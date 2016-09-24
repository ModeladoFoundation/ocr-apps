
#include <ocr.h>
#include <nanos6_rt_interface.h>
#include <extensions/ocr-runtime-itf.h>

#include "common.h"
#include "dependences.h"
#include "hashtable.h"
#include "task.h"

/*! \brief Register a task read access on linear range of addresses
 *
 *  Procedure:
 *  Read only dependences make use of two different OCR events:
 *   - 1 latch event: used to mark a read=only section, which consists
 *   on a group of read only EDTs that are able to run simultaneously.
 *   Writer EDTs following this read-only section must synchronize with
 *   all previous readers.
 *   - 1 sticky event: latch events are self destroyed, therefore there is
 *   no way of ensuring whether a writer EDT has its dependences satisfied
 *   if they are not available any more.
 *
 *  \param[in] handler the handler received in register_depinfo
 *  \param[in] start first address accessed
 *  \param[in] length number of bytes until and including the last byte accessed
 */
void nanos_register_read_depinfo(void *handler, void *start, size_t length)
{
    task_definition_t* taskdef = (task_definition_t*)handler;

    // Workaround to get a pointer from EDT local storage
    union _els_to_ptr tmp;
    tmp.els = ocrElsUserGet( 0U );
    hash_table_t* dependences = (hash_table_t*)tmp.ptr;

    data_dependency_t* dep_data = hashTableGet( dependences, start );
    // Add read-after-write dependency to previous writer EDTs
    addDependencyRAW( taskdef, dep_data );
    // Create read-only section and register read operation
    createReadSection( dep_data );
    readSectionAddReader( taskdef, dep_data );
}

/*! \brief Register a task write access on linear range of addresses
 *
 *  This (simplistic) implementation makes no difference between
 *  write only dependences and read-write dependences.
 *  The possible optimization of write only dependences would be to
 *  avoid WaW (write after write) dependences, which can be solved using
 *  renaming. This is not supported yet.
 *  Therefore, there is no difference, at the moment, between calling
 *  nanos_register_write_depinfo or nanos_register_readwrite_depinfo.
 *
 *  \param[in] handler the handler received in register_depinfo
 *  \param[in] start first address accessed
 *  \param[in] length number of bytes until and including the last byte accessed
 */
void nanos_register_write_depinfo(void *handler, void *start, size_t length)
{
    nanos_register_readwrite_depinfo( handler, start, length );
}

/*! \brief Register a task read and write access on linear range of addresses
 *
 *  Procedure:
 *  Read-write EDTs must synchronize with both read-only and other writer EDTs.
 *  There is a difference with read-only EDTs, though: using a latch event is
 *  not necessary in this case, since only a single EDT can run in a read-write
 *  section.
 *
 *  \param[in] handler the handler received in register_depinfo
 *  \param[in] start first address accessed
 *  \param[in] length number of bytes until and including the last byte accessed
 */
void nanos_register_readwrite_depinfo(void *handler, void *start, size_t length)
{
    task_definition_t* taskdef = (task_definition_t*)handler;

    // Workaround to get a pointer from EDT local storage
    union _els_to_ptr tmp;
    tmp.els = ocrElsUserGet( 0U );
    hash_table_t* dependences = (hash_table_t*)tmp.ptr;

    data_dependency_t* dep_data = hashTableGet( dependences, start );
    // Add write-after-write dependency to previous writer EDTs
    addDependencyWAW( taskdef, dep_data );
    // Create write-section
    createWriteSection( dep_data );
    // Add write-after-read dependency to previous reader EDTs
    addDependencyWAR( taskdef, dep_data );
}

