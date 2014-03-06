/*
 * gfcompress.h - compress data
 * Writen by CX3201 2013-05-29
 */

#ifndef __GFCOMPRESS_H__
#define __GFCOMPRESS_H__

#include <stdint.h>

#define GF_COMPRESS_TYPE "gzip"

/* compress the source buffer into the destination buffer */
int gf_compress_data(char *dest, uint64_t *destlen, 
		const char *source, uint64_t srclen, int level);

/* decompress the source buffer into the destination buffer */
int gf_uncompress_data(char *dest, uint64_t *destlen, 
				const char *source, uint64_t srclen);

#endif /* __GFCOMPRESS_H__ */
