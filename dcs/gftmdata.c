/*
 * gftmdata.c - GF data interface
 *
 * Writen by CX3201 2013-07-04
 */

#include "gftmdata.h"
#include "gferror.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

int gf_tm_get_block(uint8_t *buf, char *sat, char *orb, char *chn, char *filename, int64_t offset, int64_t size, int check, int compress)
{
	int64_t sret;
	int fd;
	char path[1024];

	sprintf(path, "testfile/%s/%s/%s", orb, chn, filename);
	fd = open(path, O_RDONLY);
	if(-1 == fd) return -1;
	sret = pread(fd, buf, size, offset);
	if(sret != size){
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gf_tm_poll_sat(char *sat, int (*fn)(char *sat, char *orb))
{
	DIR *pdir;
	struct dirent *p;

if(strcmp(sat, "GF01") != 0) return 0;
	pdir = opendir("testfile");
	if(!pdir) GF_ERROR(GFOPENERR);
	while(1){
		p = readdir(pdir);
		if(!p) break;
		if(strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) continue;
		if(fn(sat, p->d_name)){
			closedir(pdir);
			GF_ERROR(errno);
		}
	}
	closedir(pdir);
	return 0;
}

int gf_tm_poll_task(char *sat, char *orb, int (*fn)(char *sat, char *orb, char *chn))
{
	DIR *pdir;
	struct dirent *p;
	char path[1024];

	sprintf(path, "testfile/%s", orb);
	pdir = opendir(path);
	if(!pdir) GF_ERROR(GFOPENERR);
	while(1){
		p = readdir(pdir);
		if(!p) break;
		if(strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) continue;
printf("Add chn: %s-%s-%s\n", sat, orb, p->d_name);
		if(fn(sat, orb, p->d_name)){
			closedir(pdir);
			GF_ERROR(errno);
		}
	}
	closedir(pdir);
	return 0;
}

int gf_tm_poll_chn(char *sat, char *orb, char *chn, int (*fn)(char *sat, char *orb, char *chn, char *filename, int64_t size))
{
	DIR *pdir;
	struct dirent *p;
	char path[1024];
	struct stat stat;

	sprintf(path, "testfile/%s/%s", orb, chn);
	pdir = opendir(path);
	if(!pdir) GF_ERROR(GFOPENERR);
	while(1){
		p = readdir(pdir);
		if(!p) break;
		if(strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) continue;
printf("Add file: %s-%s-%s-%s\n", sat, orb, chn, p->d_name);
		sprintf(path, "testfile/%s/%s/%s", orb, chn, p->d_name);
		if(lstat(path, &stat)) GF_ERROR(GFOPENERR);
		if(fn(sat, orb, chn, p->d_name, stat.st_size)){
			closedir(pdir);
			GF_ERROR(errno);
		}
	}
	closedir(pdir);
	return 1;
}

