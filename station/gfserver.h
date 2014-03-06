/*
 * gfserver.h - GF server interface
 *
 * Writen by CX3201 2013-05-29
 */

#ifndef __GFSERVER_H__
#define __GFSERVER_H__

//#include "gf_req_handler.h"
#include "gfconfig.h"
#include <stdint.h>

#define GF_MAX_REQ_TYPE_LEN  16
#define GF_MAX_REQ_LINE_LEN  1024
#define GF_MAX_REQ_PARAM_LEN 256

/* Request */
typedef struct __gf_request_t{
	char    type[GF_MAX_REQ_TYPE_LEN];
	char    satelite[GF_MAX_REQ_PARAM_LEN];
	char    orbit[GF_MAX_REQ_PARAM_LEN];
	char    channel[GF_MAX_REQ_PARAM_LEN];
	char    file[GF_MAX_REQ_PARAM_LEN];
	char	level[GF_MAX_REQ_PARAM_LEN];
	int64_t offset;
	int64_t size;
	int	check;
	int	compress;
} gf_request_t;

typedef int (* Handler)(int, gf_request_t*);

/* Request handler table, table must end with {"",NULL} */
typedef struct __gf_req_handler_t{
	char type[GF_MAX_REQ_TYPE_LEN];
	int (* handler)(int, gf_request_t*);
} gf_req_handler_t;

extern gf_req_handler_t gf_req_handler_table[];  /* gf_req_handler.c */

/* Server config */
extern gf_server_conf_t    gf_server_conf;  /* gfserver.c */
extern gf_server_satconf_t *gf_server_satconf;  /* gfserver.c */

/* Initialize server */
int   gf_init_server(void);

/***********Local function*************/
/* Initialize request handle thread */
void* gf_req_thread(void *listenfd);
int   gf_make_thread(int listenfd);
/* Get request */
int   gf_get_request(int fd, gf_request_t *req);
/* Request ip check */
int   gf_ip_check(gf_request_t *req, char *ip);
/* Lookup handler table */
Handler gf_lookup_handler(char *req_type);

#endif /* __GFSERVER_H__ */
