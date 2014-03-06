/*
 * gftmdata.h - GF data interface
 *
 * Writen by CX3201 2013-07-04
 */

#ifndef __GFTMDATA_H__
#define __GFTMDATA_H__

#include <stdint.h>

int gf_tm_get_block(uint8_t *buf, char *sat, char *orb, char *chn, char *filename, int64_t offset, int64_t size, int check, int compress);

int gf_tm_poll_sat(char *sat, int (*fn)(char *sat, char *orb));
int gf_tm_poll_task(char *sat, char *orb, int (*fn)(char *sat, char *orb, char *chn));
/* 0-success >0-complete -1-error */
int gf_tm_poll_chn(char *sat, char *orb, char *chn, int (*fn)(char *sat, char *orb, char *chn, char *filename, int64_t size));

#endif /* GFTMDATA */

