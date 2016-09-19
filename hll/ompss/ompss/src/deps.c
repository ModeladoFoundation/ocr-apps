
#include <nanos6_rt_interface.h>

/*! \brief Register a task read access on linear range of addresses
 *
 *  \param[in] handler the handler received in register_depinfo
 *  \param[in] start first address accessed
 *  \param[in] length number of bytes until and including the last byte accessed
 */
void nanos_register_read_depinfo(void *handler, void *start, size_t length)
{
}

/*! \brief Register a task write access on linear range of addresses
 *
 *  \param[in] handler the handler received in register_depinfo
 *  \param[in] start first address accessed
 *  \param[in] length number of bytes until and including the last byte accessed
 */
void nanos_register_write_depinfo(void *handler, void *start, size_t length)
{
}

/*! \brief Register a task read and write access on linear range of addresses
 *
 *  \param[in] handler the handler received in register_depinfo
 *  \param[in] start first address accessed
 *  \param[in] length number of bytes until and including the last byte accessed
 */
void nanos_register_readwrite_depinfo(void *handler, void *start, size_t length)
{
}

