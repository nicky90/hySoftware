/*
 * gftmstatusfile.h - GF file transmit status
 *
 * Writen by CX3201 2013-06-08
 */

#ifndef __GFTMSTATUSFILE_H__
#define __GFTMSTATUSFILE_H__

#include "gftm.h"
#include <stdint.h>

#define GF_TM_FILE_MODE  0666
#define GF_TM_DIR_MODE   0777

int gf_tm_make_file_path(char *path, char *sat, char *orb, char *chn, char *filename, char *suffix);
int gf_tm_open_file(char *path, int trunc);
int gf_tm_delete_path(char *path);

#endif /* __GFTMSTATUSFILE_H__ */
