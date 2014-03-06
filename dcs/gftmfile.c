/*
 * gftmstatusfile.h - GF file transmit status
 *
 * Writen by CX3201 2013-06-08
 */

#include "gftmfile.h"
#include "gftmconfig.h"
#include "gferror.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

int gf_tm_make_file_path(char *path, char *sat, char *orb, char *chn, char *filename, char *suffix)
{
	int pos = 0, i = 0;
	gf_tm_sat_config_t *psatconf = NULL;

	psatconf = gf_tm_find_sat_config(sat);
	if(!psatconf) GF_ERROR(GFNEXIST);
	i = 0;
	while(psatconf->root[i]) path[pos++] = psatconf->root[i++];
	if(sat && psatconf->satdir){
		i = 0;
		while(sat[i]) path[pos++] = sat[i++];
		path[pos++] = '/';
	}
	if(orb && psatconf->orbdir){
		i = 0;
		while(orb[i]) path[pos++] = orb[i++];
		path[pos++] = '/';
	}
	if(chn && psatconf->chndir){
		i = 0;
		while(chn[i]) path[pos++] = chn[i++];
		path[pos++] = '/';
	}
	i = 0;
	if(filename) while(filename[i]) path[pos++] = filename[i++];
	if(suffix){
		i = 0;
		while(suffix[i]) path[pos++] = suffix[i++];
	}
	path[pos] = '\0';

	return 0;
}

/* -1-error else fd*/
int gf_tm_open_file(char *path, int trunc)
{
	int   fd;
	char *p = path + 1;
	struct stat st;

	if(!path) GF_ERROR(GFNULLPARAM);
	if(path[0] != '/') GF_ERROR(GFINVAPARAM);

	if(trunc) trunc = O_TRUNC;
	else trunc = 0;
	while(1){
		while(*p && *p != '/') p++;
		if(!*p){
			if(lstat(path, &st)){
				if(ENOENT != errno) GF_ERROR(GFOPENERR);
			}else if(!S_ISREG(st.st_mode)) GF_ERROR(GFOPENERR)
			fd = open(path, O_RDWR|O_CREAT|trunc, GF_TM_FILE_MODE);
			if(-1 == fd) GF_ERROR(GFOPENERR);
			return fd;
		}
		/* *p == '/' */
		*p = '\0';
		if(lstat(path, &st)){
			if(ENOENT == errno){
				if(mkdir(path, GF_TM_DIR_MODE)){
					*p = '/';
					GF_ERROR(GFCREATEERR);
				}
			}else{
				*p = '/';
				GF_ERROR(GFCREATEERR);
			}
		}else if(!S_ISDIR(st.st_mode)){
			*p = '/';
			GF_ERROR(GFCREATEERR);
		}
		*p = '/';
		p++;
	}
	return 0;
}

int gf_tm_delete_path(char *path)
{
	int tmp, len;
	char chdpath[GF_TM_PATH_MAX+1];
	DIR *pdir;
	struct dirent *p;
	struct stat st;
	
	if(!path) GF_ERROR(GFNULLPARAM);
	len = strlen(path);
	if(len < 1) GF_ERROR(GFINVAPARAM);
	if(lstat(path, &st)){
		if(ENOENT == errno) return 0;
		GF_ERROR(GFOPENERR);
	}
	if(!S_ISDIR(st.st_mode)){
		if(remove(path)) GF_ERROR(GFDELERR);
		return 0;
	}
	pdir = opendir(path);
	if(!pdir) GF_ERROR(GFOPENERR);
	while(1){
		p = readdir(pdir);
		if(!p) break;
		if(strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) continue;
		if('/' == path[len-1] || '\\' == path[len-1])
			sprintf(chdpath, "%s%s", path, p->d_name);
		else sprintf(chdpath, "%s/%s", path, p->d_name);
		if(gf_tm_delete_path(chdpath)){
			tmp = errno;
			closedir(pdir);
			GF_ERROR(tmp);
		}
	}
	closedir(pdir);
	if(remove(path)) GF_ERROR(GFDELERR);

	return 0;
}

