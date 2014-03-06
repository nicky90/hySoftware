/*
 * gftmconfig.h - GF task manager configuration
 *
 * These functions are not threads safe
 *
 * Writen by CX3201 2013-06-13
 */

#include "gftmconfig.h"
#include "gferror.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

gf_tm_config_t      gf_tm_config = GF_TM_DEFAULT_CONFIG;
gf_tm_sat_config_t  gf_tm_default_sat_config_temp = GF_TM_DEFAULT_SAT_CONFIG;
gf_tm_sat_config_t *pgf_tm_sat_config = NULL;

static int gf_tm_get_config_line(FILE *file, int *ptype, char *seg, char *key, char *value)
{
	int  i, ch, len, quote = 0;
	char line[GF_TM_CONF_STR_MAX+1];

	if(!file) GF_ERROR(GFNULLPARAM);

	while(1){
		for(i = 0; i < GF_TM_CONF_STR_MAX; i++){
			ch = fgetc(file);
			if(EOF == ch){
				if(0 == i){
					*ptype = GF_TM_CONF_EOF;
					return 0;
				}
				break;
			}
			if('\n' == ch || '#' == ch) break;
			if('\'' == ch) quote = !quote;
			if(isgraph(ch)) line[i] = (char)ch;
			else if(quote) line[i] = ' ';
			else i--;
		}
		line[i] = '\0';
		while('\n' != ch) ch = fgetc(file);
		quote = 0;
		if('\0' == line[0]) continue;
		break;
	}
	len = strlen(line);
	if('[' == line[0]){
		if(len < 3 || ']' != line[len - 1]) GF_ERROR(GFCONFERR)
		else line[len - 1] = '\0';
		strcpy(seg, line + 1);
		*ptype = GF_TM_CONF_SEG;
	}else{
		for(i = 0; i < len; i++) if('=' == line[i]) break;
		if(i == len) GF_ERROR(GFCONFERR);
		line[i++] = '\0';
		if('\'' == line[i] && '\'' == line[len-1]){
			i++;
			line[len-1] = '\0';
		}
		strcpy(key, line);
		strcpy(value, line + i);
		*ptype = GF_TM_CONF_KEYVAL;
	}
	return 0;
}

gf_tm_sat_config_t* gf_tm_find_sat_config(char *sat)
{
	gf_tm_sat_config_t *p = pgf_tm_sat_config;

	if(!sat) return NULL;
	while(p){
		if(strcmp(p->name, sat) == 0) return p;
		p = p->next;
	}
	return NULL;
}

int gf_tm_read_sat_config(char *config_file)
{
	int  type, tmp;
	char seg[GF_TM_CONF_STR_MAX+1];
	char key[GF_TM_CONF_STR_MAX+1];
	char value[GF_TM_CONF_STR_MAX+1];
	FILE *pfile = NULL;
	gf_tm_sat_config_t *p = NULL;

	pfile = fopen(config_file, "r");
	if(!pfile) GF_ERROR(GFNEXIST);
	while(1){
		if(gf_tm_get_config_line(pfile, &type, seg, key, value)){
			tmp = errno;
			fclose(pfile);
			GF_ERROR(tmp);
		}
		if(GF_TM_CONF_EOF == type){
			if(p){
				if('\0' == p->root[0] || gf_tm_find_sat_config(p->name)){
					tmp = GFCONFERR;
					fclose(pfile);
					free(p);
					GF_ERROR(tmp);
				}
				p->next = pgf_tm_sat_config;
				pgf_tm_sat_config = p;
			}
			p = NULL;
			break;
		}
		if(GF_TM_CONF_SEG == type){
			if(p){
				if('\0' == p->root[0] || gf_tm_find_sat_config(p->name)){
					tmp = GFCONFERR;
					fclose(pfile);
					free(p);
					GF_ERROR(tmp);
				}
				p->next = pgf_tm_sat_config;
				pgf_tm_sat_config = p;
			}
			p = (gf_tm_sat_config_t*)malloc(sizeof(gf_tm_sat_config_t));
			if(!p){
				tmp = GFALLOCERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			memcpy(p, &gf_tm_default_sat_config_temp, sizeof(gf_tm_sat_config_t));
			strncpy(p->name, seg, GF_TM_SAT_NAME_MAX+1);
			continue;
		}
		if(!p){
			tmp = GFALLOCERR;
			fclose(pfile);
			GF_ERROR(tmp);
		}
		if(strcasecmp(key, "server") == 0){
			if(inet_aton(value, (void*)&(p->server)) <= 0){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "port") == 0){
			if(1 != sscanf(value, "%d", &(p->port))){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
			if(!GF_TM_VALID_PORT(p->port)){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "root") == 0){
			if(value[0] != '/'){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
			strncpy(p->root, value, GF_TM_PATH_MAX+1);
			tmp = strlen(p->root);
			if('/' != p->root[tmp-1]){
				if(tmp >= GF_TM_PATH_MAX){
					tmp = GFCONFERR;
					fclose(pfile);
					free(p);
					GF_ERROR(tmp);
				}
				p->root[tmp] = '/';
				p->root[tmp+1] = '\0';
			}
		}else if(strcasecmp(key, "chn_num") == 0){
			if(1 != sscanf(value, "%d", &(p->chn_num))){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
			if(p->chn_num <= 0){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "priority") == 0){
			if(1 != sscanf(value, "%d", &(p->priority))){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
			if(p->priority <= 0){
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "check") == 0){
			if(strcasecmp(value, "yes") == 0) p->check = 1;
			else if(strcasecmp(value, "no") == 0) p->check = 0;
			else{
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "compress") == 0){
			if(strcasecmp(value, "fast") == 0) p->compress = 1;
			else if(strcasecmp(value, "no") == 0) p->compress = 0;
			else if(strcasecmp(value, "best") == 0) p->compress = 9;
			else{
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "satdir") == 0){
			if(strcasecmp(value, "yes") == 0) p->satdir = 1;
			else if(strcasecmp(value, "no") == 0) p->satdir = 0;
			else{
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}/*else if(strcasecmp(key, "orbdir") == 0){
			if(strcasecmp(value, "yes") == 0) p->orbdir = 1;
			else if(strcasecmp(value, "no") == 0) p->orbdir = 0;
			else{
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "chndir") == 0){
			if(strcasecmp(value, "yes") == 0) p->chndir = 1;
			else if(strcasecmp(value, "no") == 0) p->chndir = 0;
			else{
				tmp = GFCONFERR;
				fclose(pfile);
				free(p);
				GF_ERROR(tmp);
			}
		}*/else if(strcasecmp(key, "orbhandler") == 0){
			strncpy(p->orbhandler, value, GF_TM_PATH_MAX+1);
		}else if(strcasecmp(key, "chnhandler") == 0){
			strncpy(p->chnhandler, value, GF_TM_PATH_MAX+1);
		}else if(strcasecmp(key, "filehandler") == 0){
			strncpy(p->filehandler, value, GF_TM_PATH_MAX+1);
		}else GF_WARNING("Garbage config line");
	}

	return 0;
}

int gf_tm_read_config(char *config_file)
{
	int  type, tmp;
	char seg[GF_TM_CONF_STR_MAX+1];
	char key[GF_TM_CONF_STR_MAX+1];
	char value[GF_TM_CONF_STR_MAX+1];
	FILE *pfile = NULL;

	pfile = fopen(config_file, "r");
	if(!pfile) GF_ERROR(GFNEXIST);
	while(1){
		if(gf_tm_get_config_line(pfile, &type, seg, key, value)){
			tmp = errno;
			fclose(pfile);
			GF_ERROR(tmp);
		}
		if(GF_TM_CONF_EOF == type) break;
		if(GF_TM_CONF_SEG == type){
			tmp = GFCONFERR;
			fclose(pfile);
			GF_ERROR(tmp);
		}
		if(strcasecmp(key, "block_size") == 0){
			if(1 != sscanf(value, "%d", &tmp)){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			if(!GF_TM_VALID_BLOCK_SIZE(tmp)){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			gf_tm_config.block_size = (int64_t)tmp << 20;
		}else if(strcasecmp(key, "thread_num") == 0){
			if(1 != sscanf(value, "%d", &(gf_tm_config.thread_num))){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			if(!GF_TM_VALID_THREAD_NUM(gf_tm_config.thread_num)){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "timeout") == 0){
			if(1 != sscanf(value, "%d", &(gf_tm_config.timeout))){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			if(!GF_TM_VALID_TIMEOUT(gf_tm_config.timeout)){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "poll_period") == 0){
			if(1 != sscanf(value, "%d", &(gf_tm_config.poll_period))){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			if(!GF_TM_VALID_PERIOD(gf_tm_config.poll_period)){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
		}else if(strcasecmp(key, "retry") == 0){
			if(1 != sscanf(value, "%d", &(gf_tm_config.retry))){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
			if(!GF_TM_VALID_RETRY(gf_tm_config.timeout)){
				tmp = GFCONFERR;
				fclose(pfile);
				GF_ERROR(tmp);
			}
		}else GF_WARNING("Garbage config line");
	}

	return 0;
}

int gf_tm_destroy_config(void)
{
	gf_tm_sat_config_t *p = NULL;
	
	while(pgf_tm_sat_config){
		p = pgf_tm_sat_config->next;
		free(pgf_tm_sat_config);
		pgf_tm_sat_config = p;
	}
	return 0;
}

/*************DEBUG**************/
void gf_tm_print_config(void)
{
	printf("[TM CONFIG]\nblock_size: %ld\nthread_num: %d\ntimeout: %d\npoll_period: %d\nretry: %d\n",
		gf_tm_config.block_size, gf_tm_config.thread_num, gf_tm_config.timeout, gf_tm_config.poll_period, gf_tm_config.retry);
}

void gf_tm_print_sat_config(void)
{
	gf_tm_sat_config_t *p = &gf_tm_default_sat_config_temp;

	printf("[%s]\nserver: %s:%d\nroot: %s\n"\
		"satdir: %d\norbdir: %d\nchndir: %d\n"\
		"chn_num: %d\npriority: %d\ncheck: %d\ncompress: %d\n"\
		"orbhandler: %s\nchnhandler: %s\nfilehandler: %s\n",
		p->name, inet_ntoa(p->server), p->port,p->root,
		p->satdir, p->orbdir, p->chndir,
		p->chn_num, p->priority, p->check, p->compress,
		p->orbhandler, p->chnhandler, p->filehandler);
	p = pgf_tm_sat_config;
	while(p){
		printf("[%s]\nserver: %s:%d\nroot: %s\n"\
			"satdir: %d\norbdir: %d\nchndir: %d\n"\
			"chn_num: %d\npriority: %d\ncompress: %d\n"\
			"orbhandler: %s\nchnhandler: %s\nfilehandler: %s\n",
			p->name, inet_ntoa(p->server), p->port,p->root,
			p->satdir, p->orbdir, p->chndir,
			p->chn_num, p->priority, p->compress,
			p->orbhandler, p->chnhandler, p->filehandler);
		p = p->next;
	}
}

int config_main(int argc, char *argv[])
{
	if(argc == 3){
		if(gf_tm_read_config(argv[1])) GF_ERROR_PR(errno);
		if(gf_tm_read_sat_config(argv[2])) GF_ERROR_PR(errno);
	}else{
		if(gf_tm_read_config("tm.cfg")) GF_ERROR_PR(errno);
		if(gf_tm_read_sat_config("tmsat.cfg")) GF_ERROR_PR(errno);
	}
	gf_tm_print_config();
	gf_tm_print_sat_config();
	gf_tm_destroy_config();

	return 0;
}

