/*
 * gfconfig.c - set several options
 */

#include "gfconfig.h"
#include "../common/gferror.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/* removing leading and trailing whitespaces */
char *trim(const char *str, char trimed[]) {
	const char *p, *q;

	if (0 == strlen(str)) return NULL;

	/* overleap leading whitespaces, p points to first non-whitespace */
	for (p = str; *p && isspace((int)*p); p++);

	/* overleap trailing whitespaces, q points to last non-whitespace */
	for (q = p + strlen(p) - 1;
		q >= p && isspace((int)*q); q--);

	strncpy(trimed, p, q - p + 1);
	trimed[q - p + 1] = '\0';

	return trimed;
}

/* removing comments starting with '#' */
char *delcomments(const char *str, char commentless[]) {
	const char *p;

	if (0 == strlen(str)) return NULL;

	p = strchr(str, '#');
	if (!p) strcpy(commentless, str);

	strncpy(commentless, str, p - str);
	commentless[p - str] = '\0';

	return commentless;
}

int get_key_value(const char *str, char key[], char value[]) {
	char tmp_key[GF_MAX_KEY_LEN + 1] = {'\0', };
	char tmp_val[GF_MAX_VALUE_LEN + 1] = {'\0', };
	const char *p, *q;

	p = strchr(str, '=');
	if (NULL == p) return 2;
	if ( strlen(str) == 0 ) return 2;
	q = str + strlen(str) - 1;

	strncpy(tmp_key, str, p - str);
	tmp_key[p - str] = '\0';
	trim(tmp_key, key);

	strncpy(tmp_val, p + 1, q - p);
	tmp_val[q - p] = '\0';
	trim(tmp_val, value);

	return 0;
}

/* Read server config from config file */
int gf_init_server_config(const char *config_file, \
			gf_server_conf_t *server_conf) {

	char trimed_buf[GF_MAX_LINE_LEN + 1] = {'\0', };
//char cmtless_buf[GF_MAX_LINE_LEN + 1] = {'\0', };
	char linebuffer[GF_MAX_LINE_LEN + 1] = {'\0', };
	char config_key[GF_MAX_KEY_LEN + 1] = {'\0', };
	char config_value[GF_MAX_VALUE_LEN + 1] = {'\0', };
	int ret;

	server_conf->port = 4746;
	server_conf->max_thread = 64;
	server_conf->timeout = 100;


	FILE *file = fopen(config_file, "r");

	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if ((ret = get_key_value(trimed_buf, \
					config_key,  \
					config_value)) == 2)
			continue;
		if (ret == 0) {
			if (strlen(config_value) == 0) continue;

			if (!strncmp(config_key, "port", 4)) {
				server_conf->port = atoi(config_value);
			}
			else if (!strncmp(config_key, "link", 4)) {
				server_conf->max_thread = atoi(config_value);
			}
			else if (!strncmp(config_key, "timeout", 7)) {
				server_conf->timeout = atoi(config_value);
			}
			else continue; 
		}

	}
	fclose(file);

	return 0;
}

#if 0

/* Read server satelite config from config file */
int gf_init_server_satellite(const char *config_file, 
				gf_server_satconf_t *server_satconf)
{
	char trimed_buf[GF_MAX_LINE_LEN + 1] = {'\0', };
	char cmtless_buf[GF_MAX_LINE_LEN + 1] = {'\0', };
	char linebuffer[GF_MAX_LINE_LEN + 1] = {'\0', };
	char config_key[GF_MAX_KEY_LEN + 1] = {'\0', };
	char config_value[GF_MAX_VALUE_LEN + 1] = {'\0', };
	int ret;
	char *ptr;
	char name[GF_MAX_PATH_LEN] = {'\0', };

	gf_server_satconf_t *p, *q, *r;
	p = server_satconf;
	q = server_satconf->next;
	r = server_satconf->next->next;
	r->next = NULL;

	strcpy(p->name, "GF01");
	strcpy(p->root, "/home/nsoas/DATA/GF01");
	p->check = 0;
	p->compress = 1;
//	strcpy(p->ip_filter->ip, "10.12.117.201");    //allow BJ server's IP
	p->ip_filter->next = NULL;

	strcpy(q->name, "GF02");
	strcpy(q->root, "/home/nsoas/DATA/GF02");
	q->check = 0;
	q->compress = 1;
//	strcpy(p->ip_filter->ip, "10.12.117.201");    //allow BJ server's IP
	q->ip_filter->next = NULL;
	
	
	strcpy(r->name, "GF03");
	strcpy(r->root, "/home/nsoas/DATA/GF03");
	r->check = 0;
	r->compress = 1;
//	strcpy(p->ip_filter->ip, "10.12.117.201");    //allow BJ server's IP
	r->ip_filter->next = NULL;

	FILE *file = fopen(config_file, "r");
	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if (trimed_buf[0] == '[') {
			ptr = strchr(trimed_buf, ']');
			if (NULL == ptr) continue;
			sscanf(trimed_buf, "[%[^]]]", name);
			if (!strncmp(name, "GF01", 4)) goto CONF_GF01;
			if (!strncmp(name, "GF02", 4)) goto CONF_GF02;
			if (!strncmp(name, "GF03", 4)) goto CONF_GF03;
			continue;
		}
		else continue;
CONF_GF01:
	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if (trimed_buf[0] == '[') {
			ptr = strchr(trimed_buf, ']');
			if (NULL == ptr) continue;
			sscanf(trimed_buf, "[%[^]]]", name);
			if (!strncmp(name, "GF01", 4)) goto CONF_GF01;
			if (!strncmp(name, "GF02", 4)) goto CONF_GF02;
			if (!strncmp(name, "GF03", 4)) goto CONF_GF03;
			continue;
		}
		ret = get_key_value(trimed_buf, config_key, config_value);
		if (2 == ret) continue;
		if (0 == ret) {
			if (strlen(config_value) == 0) continue;

			if (!strncmp(config_key, "root", 4)) {
				strcpy(p->root, config_value);
			}
			if (!strncmp(config_key, "check", 5)) {
				p->check = atoi(config_value);
			}
			if (!strncmp(config_key, "compress", 8)) {
				p->compress = atoi(config_value);
			}
			if (!strncmp(config_key, "allowfrom", 9)) {
				// reserved
			}
			continue;
		}
	}
CONF_GF02:
	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if (trimed_buf[0] == '[') {
			ptr = strchr(trimed_buf, ']');
			if (NULL == ptr) continue;
			sscanf(trimed_buf, "[%[^]]]", name);
			if (!strncmp(name, "GF01", 4)) goto CONF_GF01;
			if (!strncmp(name, "GF02", 4)) goto CONF_GF02;
			if (!strncmp(name, "GF03", 4)) goto CONF_GF03;
			continue;
		}
		ret = get_key_value(trimed_buf, config_key, config_value);
		if (2 == ret) continue;
		if (0 == ret) {
			if (strlen(config_value) == 0) continue;

			if (!strncmp(config_key, "root", 4)) {
				strcpy(q->root, config_value);
			}
			if (!strncmp(config_key, "check", 5)) {
				q->check = atoi(config_value);
			}
			if (!strncmp(config_key, "compress", 8)) {
				q->compress = atoi(config_value);
			}
			if (!strncmp(config_key, "allowfrom", 9)) {
				// reserved
			}
			continue;
		}
	}
CONF_GF03:
	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if (trimed_buf[0] == '[') {
			ptr = strchr(trimed_buf, ']');
			if (NULL == ptr) continue;
			sscanf(trimed_buf, "[%[^]]]", name);
			if (!strncmp(name, "GF01", 4)) goto CONF_GF01;
			if (!strncmp(name, "GF02", 4)) goto CONF_GF02;
			if (!strncmp(name, "GF03", 4)) goto CONF_GF03;
			continue;
		}
		ret = get_key_value(trimed_buf, config_key, config_value);
		if (2 == ret) continue;
		if (0 == ret) {
			if (strlen(config_value) == 0) continue;

			if (!strncmp(config_key, "root", 4)) {
				strcpy(r->root, config_value);
			}
			if (!strncmp(config_key, "check", 5)) {
				r->check = atoi(config_value);
			}
			if (!strncmp(config_key, "compress", 8)) {
				r->compress = atoi(config_value);
			}
			if (!strncmp(config_key, "allowfrom", 9)) {
				// reserved
			}
			continue;
		}
	}
	}
	fclose(file);

	return 0;
}

#endif
gf_server_satconf_t *server_satconf;

int gf_init_server_satellite(const char *config_file, \
			gf_server_satconf_t **server_satconf)
{
	char trimed_buf[GF_MAX_LINE_LEN + 1] = {'\0', };
//char cmtless_buf[GF_MAX_LINE_LEN + 1] = {'\0', };
	char linebuffer[GF_MAX_LINE_LEN + 1] = {'\0', };
	char config_key[GF_MAX_KEY_LEN + 1] = {'\0', };
	char config_value[GF_MAX_VALUE_LEN + 1] = {'\0', };
	int ret;
	char *ptr;
	char name[GF_MAX_PATH_LEN] = {'\0', };

	gf_server_satconf_t *p, *q, *server_satconf_head;

	// head
	server_satconf_head = (gf_server_satconf_t *)malloc(sizeof(gf_server_satconf_t));
	memset(server_satconf_head, 0, sizeof(gf_server_satconf_t));

	p = server_satconf_head;

	FILE *file = fopen(config_file, "r");
	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if (trimed_buf[0] == '[') {
			ptr = strchr(trimed_buf, ']');
			if (NULL == ptr) continue;
			sscanf(trimed_buf, "[%[^]]]", name);
			goto CONF_GF;
		}
		else continue;
CONF_GF:
	q = (gf_server_satconf_t *)malloc(sizeof(gf_server_satconf_t));
	memset(q, 0, sizeof(gf_server_satconf_t));
	q->next = NULL;
	p->next = q;
	p = q;

	strcpy(p->name, name);

	while (fgets(linebuffer, GF_MAX_LINE_LEN, file)) {
//		if (!delcomments(linebuffer, cmtless_buf)) continue;
//		if (!trim(cmtless_buf, trimed_buf)) continue;
		if (!trim(linebuffer, trimed_buf)) continue;

		if (trimed_buf[0] == '[') {
			ptr = strchr(trimed_buf, ']');
			if (NULL == ptr) continue;
			sscanf(trimed_buf, "[%[^]]]", name);
			goto CONF_GF;
		}
		ret = get_key_value(trimed_buf, config_key, config_value);
		if (2 == ret) continue;
		if (0 == ret) {
			if (strlen(config_value) == 0) continue;

			if (!strncmp(config_key, "root", 4)) {
				strcpy(p->root, config_value);
			}
			if (!strncmp(config_key, "check", 5)) {
				p->check = atoi(config_value);
			}
			if (!strncmp(config_key, "compress", 8)) {
				p->compress = atoi(config_value);
			}
			if (!strncmp(config_key, "allowfrom", 9)) {
				// reserved
			}
			continue;
		}
	}
	}
	fclose(file);

	*server_satconf = server_satconf_head->next;

	return 0;
}
