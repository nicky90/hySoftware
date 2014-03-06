/*
 * gftmconfig.h - GF task manager configuration
 *
 * These functions are not threads safe
 *
 * Writen by CX3201 2013-06-11
 */

#ifndef __GFTMCONFIG_H__
#define __GFTMCONFIG_H__

#include "gftm.h"
#include <stdio.h>
#include <arpa/inet.h>

#define GF_TM_CONF_EOF    0
#define GF_TM_CONF_SEG    1
#define GF_TM_CONF_KEYVAL 2

#define GF_TM_CONF_STR_MAX  1024
#define GF_TM_CONF_ADDR_MAX 32

#define GF_TM_DEFAULT_CONFIG     {32,4<<20,10,5,0}
#define GF_TM_DEFAULT_SAT_CONFIG {"DEFAULT",{0x0100007F},2013,"",1,1,1,2,1024,0,0,"","","",NULL}

#define GF_TM_VALID_BLOCK_SIZE(x) ((x)>=1 && (x)<=64)
#define GF_TM_VALID_THREAD_NUM(x) ((x)>=1 && (x)<=64)
#define GF_TM_VALID_TIMEOUT(x)    ((x)>=0)
#define GF_TM_VALID_RETRY(x)      ((x)>=0)
#define GF_TM_VALID_PORT(x)       ((x)>1024)
#define GF_TM_VALID_PERIOD(x)     ((x)>=5)

typedef struct __gf_tm_config_t{
	int      thread_num;
	int64_t  block_size;
	int      timeout;
	int      poll_period;
	int      retry;
} gf_tm_config_t;

typedef struct __gf_tm_sat_config_t{
	char name[GF_TM_SAT_NAME_MAX+1];
	struct in_addr server;
	int  port;
	char root[GF_TM_PATH_MAX+1];
	int  satdir;
	int  orbdir;
	int  chndir;
	int  chn_num;
	int  priority;
	int  check;
	int  compress;
	char orbhandler[GF_TM_PATH_MAX+1];
	char chnhandler[GF_TM_PATH_MAX+1];
	char filehandler[GF_TM_PATH_MAX+1];
	struct __gf_tm_sat_config_t *next;
} gf_tm_sat_config_t;

extern gf_tm_config_t     gf_tm_config;
extern gf_tm_sat_config_t *pgf_tm_sat_config;

int gf_tm_read_config(char *config_file);
int gf_tm_read_sat_config(char *config_file);
gf_tm_sat_config_t* gf_tm_find_sat_config(char *sat);
int gf_tm_destroy_config(void);

/**********DEBUG**********/
void gf_tm_print_config(void);
void gf_tm_print_sat_config(void);

#endif /* __GFTMCONFIG_H__ */

