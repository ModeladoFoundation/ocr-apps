
#include "debug.h"
#include "dependences.h"
#include "task.h"
#include "task-local.h"

#include <nanos6_rt_interface.h>
#include <ocr.h>

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
    using namespace ompss;

    Task* task = (Task*)handler;

    uintptr_t begin = reinterpret_cast<uintptr_t>(start);
    uintptr_t end   = begin + length;

    // Get running task access map (not "handler"'s)
    DependenceMap& accesses = getLocalScope().accesses;

    DependenceMap::iterator iterator = accesses.lower_bound(begin);
    if( iterator == accesses.end() || begin < iterator->first ) {
        // Entry did not exist previously
        // Note: std::piecewise_construct is a tag that indicates that
        // the std::pair in map::value_type is using perfect forwarding.
        // That is, arguments to construct each tuple are used instead
        // of passing the values themselves.
        iterator = accesses.emplace_hint( iterator,
                                          std::piecewise_construct,
                                          std::forward_as_tuple(begin),
                                          std::forward_as_tuple<>() );
        AccessDependence& dep_data = iterator->second;

        // Create read-section
        dep_data.createReadSection();
        ++iterator;
    }

    while( iterator != accesses.end() && iterator->first < end ) {
        AccessDependence& dep_data = iterator->second;

        // Add read-after-write dependency to previous writer EDTs
        dep_data.addRAWDependence( *task );
        // Create read-only section and register read operation
        dep_data.createReadSection();
        // Add EDT to the read section readers
        dep_data.readSectionAddReader( *task );

        ++iterator;
    }
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
    using namespace ompss;

    Task* task = (Task*)handler;

    uintptr_t begin = reinterpret_cast<uintptr_t>(start);
    uintptr_t end   = begin + length;

    // Get running task access map (not "handler"'s)
    auto& accesses = getLocalScope().accesses;

    DependenceMap::iterator iterator = accesses.lower_bound(begin);
    if( iterator == accesses.end() || begin < iterator->first ) {
        // Entry did not exist previously
        // Note: std::piecewise_construct is a tag that indicates that
        // the std::pair in map::value_type is using perfect forwarding.
        // That is, arguments to construct each tuple are used instead
        // of passing the values themselves.
        iterator = accesses.emplace_hint( iterator,
                                          std::piecewise_construct,
                                          std::forward_as_tuple(begin),
                                          std::forward_as_tuple<>() );
        AccessDependence& dep_data = iterator->second;

        // Create write-section
        dep_data.createWriteSection( *task );
        ++iterator;
    }

    while( iterator != accesses.end() && iterator->first < end ) {
        AccessDependence& dep_data = iterator->second;

        // Add write-after-write dependency to previous writer EDTs
        dep_data.addWAWDependence( *task );
        // Create write-section
        dep_data.createWriteSection( *task );
        // Add write-after-read dependency to previous reader EDTs
        dep_data.addWARDependence( *task );

        ++iterator;
    }
}

