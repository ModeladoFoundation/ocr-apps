#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pil_init.h"

#ifdef PIL2AFL
#include "codelet.h"
#include "rmd_afl_all.h"
int afl_main(int, char **);
#endif

#ifdef PIL2AFL
static void error(char *str)
{
	printf("%s", str);
	exit(EXIT_FAILURE);
}
#endif

void pil_init(gpp_t *index_array, gpp_t *data_array)
{
//#ifdef PIL2OCR || PIL2C
	index_array->guid = NULL_GUID;
	index_array->ptr = NULL;
	data_array->guid = NULL_GUID;
	data_array->ptr = NULL;
//#endif // PIL2OCR
#ifdef PIL2AFL
#endif // PIL2AFL
}
