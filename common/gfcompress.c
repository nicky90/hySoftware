/*
 * gfcompress.c - gf compress data routines
 */

#include "gfcompress.h"
#include "gferror.h"

#include <zlib.h>

int gf_compress_data(char *dest, uint64_t *destlen, 
		const char *source, uint64_t srclen, int level)
{
	int ret;

	if (NULL == source) {
		GF_ERROR(GFNULLPARAM);
	}

	ret = compress2((Bytef*)dest, (uLongf*)destlen,
			(const Bytef*)source, (uLongf)srclen, level);

	if ((Z_OK == ret) && (*destlen < srclen)) {
		return 0;
	}
	else {
		GF_ERROR(GFCOMPERR);
	}
}

int gf_uncompress_data(char *dest, uint64_t *destlen, 
				const char *source, uint64_t srclen)
{
	int ret;

	if (NULL == source) GF_ERROR(GFNULLPARAM);

	ret = uncompress((Bytef*)dest, (uLongf*)destlen, 
			(const Bytef*)source, (uLongf)srclen);

	if (Z_OK == ret) {	
		return 0;
	}
	else {
		GF_ERROR(GFUNCOMPERR);
	}
}

