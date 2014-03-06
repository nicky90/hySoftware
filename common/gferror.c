/*
 * gferror.c - error code and log
 *
 * Writen by CX3201 2013-05-29
 */

#include <stdlib.h>
#include "gferror.h"

/* Error code string */
static char *gf_error_str[] = GF_ERROR_STR;

/* Get error code string */
char* gf_strerr(int err)
{
	if(!GF_RET_CODE(err)) return gf_error_str[GFUNKOWEN];
	return gf_error_str[err];
}
