#ifndef __TLS_H__
#define __TLS_H__

/* Take the pointer and assign it to the TLS register */
#define __SET_TLS_REG(ptr) \
__asm__ ("bitop1  r504,%0,0,OR,64" : : "r"(ptr))

#endif
