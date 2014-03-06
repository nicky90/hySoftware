/*
 * gfconfig.h - config file interface
 *
 * Writen by CX3201 2013-05-29
 */

#ifndef __GFCONFIG_H__
#define __GFCONFIG_H__

#define GF_MAX_PATH_LEN  256
#define GF_IP_FILTER_LEN 24

#define GF_MAX_LINE_LEN  1024
#define GF_MAX_KEY_LEN   20
#define GF_MAX_VALUE_LEN 1024

/* IP filter */
typedef struct gf_ip_filter_t {
	char ip[GF_IP_FILTER_LEN];
	struct gf_ip_filter_t *next;
} gf_ip_filter_t;

/* Server config */
typedef struct __gf_server_conf_t {
	int port;
	int max_thread;
	int timeout;
} gf_server_conf_t;

/* Server satelite config */
typedef struct __gf_server_satconf_t {
	char name[GF_MAX_PATH_LEN];
	char root[GF_MAX_PATH_LEN];
	int  check;
	int  compress;
	struct gf_ip_filter_t *ip_filter;
	struct __gf_server_satconf_t *next;
} gf_server_satconf_t;



/* Read server config from config file */
int gf_init_server_config(const char *config_file, gf_server_conf_t *server_conf);

/* Read server satelite config from config file */
int gf_init_server_satellite(const char *config_file, gf_server_satconf_t **server_satconf_inout);

#endif /* __GFCONFIG_H__ */
