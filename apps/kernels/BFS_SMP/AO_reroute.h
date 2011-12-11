#include <stdint.h>

#ifndef __AO_REROUTE_H
#define __AO_REROUTE_H

uint32_t AO_compare_and_swap(volatile uint32_t* addr, uint32_t oldval, uint32_t newval);

#endif
