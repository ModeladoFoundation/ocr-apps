#include "AO_reroute.h"

uint32_t AO_compare_and_swap(volatile uint32_t* addr, uint32_t oldval, uint32_t newval)
{
  return __sync_bool_compare_and_swap(addr, oldval, newval);



}
