/*===-- xstg_sync_fetch_and_sub.c -----------------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 *===------------------------------------------------------------------------===
 *
 * This file implements __sync_fetch_and_sub_* for XSTG architecture.
 *
 *===------------------------------------------------------------------------===
 */
# define strong_alias(name, aliasname) _strong_alias(name, aliasname)
# define _strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)));


static char __sync_fetch_and_sub_1__(volatile char *a, char b, ...) {
  return __sync_fetch_and_add_1(a, -b);
}

static short __sync_fetch_and_sub_2__(volatile short *a, short b, ...) {
  return __sync_fetch_and_add_2(a, -b);
}

static int __sync_fetch_and_sub_4__(volatile int *a, int b, ...) {
  return __sync_fetch_and_add_4(a, -b);
}

static long long __sync_fetch_and_sub_8__(volatile long long *a, long long b, ...) {
  return __sync_fetch_and_add_8(a, -b);
}

strong_alias(__sync_fetch_and_sub_1__, __sync_fetch_and_sub_1)
strong_alias(__sync_fetch_and_sub_2__, __sync_fetch_and_sub_2)
strong_alias(__sync_fetch_and_sub_4__, __sync_fetch_and_sub_4)
strong_alias(__sync_fetch_and_sub_8__, __sync_fetch_and_sub_8)
