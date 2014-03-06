/*
 * gfchecksum.h - get checksum of data
 * Writen by CX3201 2013-05-29
 */

#ifndef __GFCHECKSUM_H__
#define __GFCHECKSUM_H__

#include <inttypes.h>

#define GF_CHECK_TYPE "md5"
#define GF_CHECK_LEN  16

int gf_check_data(char *chksm, char *data, int len);
int gf_checkcmp_data(char *chksm, char *data, int len);
void gf_md5_16to32(uint8_t *str16, char *str32);
void gf_md5_32to16(char *str32, uint8_t *str16);

#endif /* __GFCHECKSUM_H__ */
