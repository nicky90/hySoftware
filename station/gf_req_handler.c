/*
 * gf_req_handler.c - request handler table and functions
 *
 * Writen by CX3201 2013-05-29
 */

#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
//#include <asm/stat.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "gfserver.h"
#include "../common/gfsock.h"
#include "../common/gferror.h"
#include "../common/gfchecksum.h"
#include "../common/gfcompress.h"
#include "gfconfig.h"

#define GF_MAX_BLOCKLEN (1<<22)

#define Gf_writeline(fd, line, num)                  \
	do {                                         \
		if(gf_writeline(fd, line, num) <0 )  \
			GF_PRINTERR(errno);          \
	} while (0);

int get(int fd, gf_request_t *request);
int list(int fd, gf_request_t *request);
int unknown(int fd, gf_request_t *request);

/* Request handler table */
gf_req_handler_t
gf_req_handler_table[] = {
	{"GET", get},
	{"LIST", list},
	{"UNKNOWN",unknown},
	{"", NULL}
};


/* get method - handle client's 'GET' requests */
int get(int fd, gf_request_t* request)
{
	int filefd;
	int cflag = 0, zflag = 0;
	ssize_t  Rsize = -1;
	uint64_t Zsize = 0;

	gf_server_satconf_t* pre_satconf;
	char fullpath[GF_MAX_PATH_LEN] = {'\0'};
	char linetmp[GF_MAX_LINE_LEN] = {'\0'};

	char presum[20];
	char *transdata, *ztransdata;

	/*---------------PRINT*MSG----------------------------
	printf("Executing from fd[%d]\n", fd);
	printf("%16s: [%s]\n", "type", request->type);
        printf("%16s: [%s]\n", "satelite", request->satelite);
        printf("%16s: [%s]\n", "orbit", request->orbit);
        printf("%16s: [%s]\n", "channel", request->channel);
        printf("%16s: [%s]\n", "file", request->file);
        printf("%16s: [%lld]\n", "offset", request->offset);
        printf("%16s: [%lld]\n", "size", request->size);
	*------------------------------------------------------*/

	/* validate the request parameters */
	if( strlen(request->satelite)==0 || 
		strlen(request->orbit)==0 || 
		strlen(request->channel)==0 || 
		strlen(request->file)==0 || 
		request->offset<0 || 
		request->size<0	)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRESEGNOFULL);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return 0;
	}
	

	pre_satconf = gf_server_satconf;
	while(pre_satconf != NULL && strcmp(request->satelite, pre_satconf->name) != 0)
		pre_satconf = pre_satconf->next;
	if (NULL == pre_satconf)
	{
		/* satellite config not found*/
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRESATNOFIND);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return 0;
	}

	/* satellite config found, the request file is in path of 'fullpath' */
	strcpy(fullpath, pre_satconf->root);
	if(fullpath[strlen(pre_satconf->root)-1] != '/')
		fullpath[strlen(pre_satconf->root)] = '/';
	strcat(fullpath, request->orbit);
	strcat(fullpath, "/");
	strcat(fullpath, request->channel);
	strcat(fullpath, "/");
	strcat(fullpath, request->file);

	/* printf("needfile[%s]\n",fullpath); */

	filefd = open(fullpath, O_RDONLY);
	if (filefd < 0)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFREOPENFIERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return -1;
	}
	if (lseek(filefd,request->offset, SEEK_SET) < 0)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRESEEKFIERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		close(filefd);
		return -1;
	}
	transdata = (char *)malloc(request->size);
	if ((Rsize=gf_readn(filefd, transdata, request->size)) < 0)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFREREADFIERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		close(filefd);
		free(transdata);
		return -1;
	}
	close(filefd);

	if (Rsize != request->size)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRERDSIZEERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		free(transdata);
		return -1;
	}	
	Gf_writeline(fd, "RESPOND GET", 11);
	//printf("line[%s]data[%s]\n",linetmp,transdata);

	memset(linetmp, 0, GF_MAX_LINE_LEN);
	sprintf(linetmp, "ORGSIZE %d", (int)Rsize);
	Gf_writeline(fd, linetmp, strlen(linetmp));

	/* checksum or not*/
	cflag = request->check > pre_satconf->check ? request->check : pre_satconf->check;
	if(cflag > 0)
	{
		//int i;
		char md32[40];

		gf_check_data(presum, transdata, (int)Rsize);	
		gf_md5_16to32((uint8_t *)presum, md32);

		memset(linetmp, '\0', GF_MAX_LINE_LEN);
		sprintf(linetmp, "MD5SUM %s",md32);
		Gf_writeline(fd, linetmp, strlen(linetmp));

//		int i=request->offset/GF_MAX_BLOCKLEN;	
	//	printf("block[%d]md5[%s]",i,md32);
		}

	zflag = ( request->compress > pre_satconf->compress ? request->compress:pre_satconf->compress);
	if(0 < zflag)
		{
		ztransdata = (char *)malloc(request->size);
		Zsize = request->size;
		}
	
	if(0 < zflag && 0 == gf_compress_data(ztransdata, &Zsize, transdata, (uint64_t)Rsize, zflag))
		{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp, "ZSIZE %llu",(unsigned long long)Zsize);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp, "COMPRESS %d", zflag);
		Gf_writeline(fd, linetmp, strlen(linetmp));

		Gf_writeline(fd, linetmp, 0);
		
		if(gf_writen(fd, ztransdata, Zsize) < 0)
			GF_PRINTERR(errno);
		Gf_writeline(fd, linetmp, 0);
	}
	else
	{	
		Gf_writeline(fd, linetmp, 0);
		
		if(gf_writen(fd, transdata, request->size) < 0)
			GF_PRINTERR(errno);
		Gf_writeline(fd, linetmp, 0);
	}
	//printf("comp[%d]Zsize[%llu],Rsize[%d]datalen[%d][%s]\n",pre_satconf->compress,Zsize,Rsize,strlen(ztransdata), ztransdata);
	if(zflag > 0) 
		free(ztransdata);
	free(transdata);
	return 0;
}


/* list method - handle client's 'LIST' requests */
int list(int fd, gf_request_t* request)
{
	int i,lflag=0;
	DIR *predir;
	struct dirent dir;
	struct dirent *dirp;
	struct stat buf,bufok;
	gf_server_satconf_t* pre_satconf;
	pre_satconf = gf_server_satconf;

	/*****PRINT***MSG***********************************
	printf("Executing from fd[%d]\n",fd);
	printf("%16s: [%s]\n","type",request->type);
        printf("%16s: [%s]\n","level",request->level);
        printf("%16s: [%s]\n","satelite",request->satelite);
        printf("%16s: [%s]\n","orbit",request->orbit);
        printf("%16s: [%s]\n","channel",request->channel);
	***************************************/
	char fullpath[GF_MAX_PATH_LEN];
	char pathtmp[GF_MAX_PATH_LEN];
	char linetmp[GF_MAX_LINE_LEN];
	memset(linetmp, 0, GF_MAX_LINE_LEN);
	memset(pathtmp, 0, GF_MAX_PATH_LEN);


	/************level = sat**********************/
	if (0 == strcmp(request->level, "sat"))
	{
		lflag = 1;
		Gf_writeline(fd, "RESPOND LIST", 12);
		Gf_writeline(fd, linetmp, 0);

		do {
			strcpy(linetmp, pre_satconf->name);
		  	Gf_writeline(fd, linetmp, strlen(linetmp));
		   	pre_satconf=pre_satconf->next;
			memset(linetmp, 0, GF_MAX_LINE_LEN);
		} while (pre_satconf != NULL);
		Gf_writeline(fd, linetmp, 0);
		return 0;
	}


	/************get satconf***********************************/
	while(pre_satconf != NULL && strcmp(request->satelite, pre_satconf->name) != 0)
	{
		pre_satconf = pre_satconf->next;
		//printf("name[%s]\n", pre_satconf->name);
	}
	printf("name[%s]level[%s]", pre_satconf->name,request->level);
		
	/***************no satconf**********************************************/
	if (NULL == pre_satconf)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRESATNOFIND);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return 0;
	}

	strcpy(fullpath, pre_satconf->root);
	if(fullpath[strlen(pre_satconf->root)-1] != '/')
		{
		fullpath[strlen(pre_satconf->root)] = '/';
		fullpath[strlen(pre_satconf->root)+1] = 0;
		}
		
	/***************level orb, chn or file, store the list fullpath first********************************************/
	if (0 == strcmp(request->level, "orb"))
	{
		lflag=1;
		if (strlen(request->satelite) == 0)	
		{
			memset(linetmp, 0, GF_MAX_LINE_LEN);
			sprintf(linetmp,"RESPOND WRONG%d", GFRESEGNOFULL);
			Gf_writeline(fd, linetmp, strlen(linetmp));
			Gf_writeline(fd, linetmp, 0);
			return -1;
		}
			
	}
	else if (0 == strcmp(request->level, "chn"))
	{
		lflag=1;
		if (strlen(request->satelite) == 0 || strlen(request->orbit) == 0)	
		{
			memset(linetmp, 0, GF_MAX_LINE_LEN);
			sprintf(linetmp,"RESPOND WRONG%d", GFRESEGNOFULL);
			Gf_writeline(fd, linetmp, strlen(linetmp));
			Gf_writeline(fd, linetmp, 0);
			return -1;
		}
		strcat(fullpath, request->orbit);
		strcat(fullpath, "/");
	}
	else if (0 == strcmp(request->level, "file"))
	{
		lflag=1;
		if (strlen(request->satelite) == 0 ||strlen(request->orbit)== 0 || strlen(request->channel) == 0)	
		{
			memset(linetmp, 0, GF_MAX_LINE_LEN);
			sprintf(linetmp,"RESPOND WRONG%d", GFRESEGNOFULL);
			Gf_writeline(fd, linetmp, strlen(linetmp));
			Gf_writeline(fd, linetmp, 0);
			return -1;
		}
		strcat(fullpath, request->orbit);
		strcat(fullpath, "/");
		strcat(fullpath, request->channel);
		strcat(fullpath, "/");
	}

	if(0 == lflag)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRELEVELERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return -1;
	}
	
	printf("fullpath[%s]\n",fullpath);
	if(lstat(fullpath, &buf) < 0)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRESTATFIERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		    printf("%16s: [%s]\n","type",request->type);
	        printf("%16s: [%s]\n","level",request->level);
        	printf("%16s: [%s]\n","satelite",request->satelite);
        	printf("%16s: [%s]\n","orbit",request->orbit);
        	printf("%16s: [%s]\n","channel",request->channel);

		return -1;
	}
	if(S_ISDIR(buf.st_mode) == 0)
	{
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFRESTATDIERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return -1;
	}
	memset(&buf, 0, sizeof(buf));

	if((predir = opendir(fullpath))==NULL) 
		{
		perror("open()");
		memset(linetmp, 0, GF_MAX_LINE_LEN);
		sprintf(linetmp,"RESPOND WRONG%d", GFREOPENDIERR);
		Gf_writeline(fd, linetmp, strlen(linetmp));
		Gf_writeline(fd, linetmp, 0);
		return -1;
		}
	Gf_writeline(fd, "RESPOND LIST", 12);
	Gf_writeline(fd, linetmp, 0);

	//while((dirp = readdir(predir)) != NULL)
	while(readdir_r(predir, &dir, &dirp) == 0 && dirp !=NULL)
	{
		//printf("read[%s]\n",dirp->d_name);
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
			continue;
		strcpy(pathtmp,fullpath);
		strcat(pathtmp,dirp->d_name);
		if (lstat(pathtmp, &buf)<0) {
//			printf("pathtmp[%s]",pathtmp);
//			perror("haha");
//			GF_ERROR(GFUNKOWEN);// get stat error
			continue;
			}
		if (S_ISREG(buf.st_mode)&& 0 == strcmp(request->level, "file"))
			{
				
			strcat(pathtmp, ".ok");
			if(strncmp(dirp->d_name,"finish",6) == 0)
				strcat(linetmp, dirp->d_name);	
			else if(lstat(pathtmp, &bufok) < 0) 
				continue;
			else
				{
				sprintf(linetmp, "%lld ",(long long)buf.st_size);
				strcat(linetmp, dirp->d_name);
				}
		}
		/* orb orb chn level */
		else if (S_ISDIR(buf.st_mode)) 
		{
			for(i=0; i<strlen(dirp->d_name); i++) 
				if(dirp->d_name[i]<'0'||dirp->d_name[i]>'9')
					break;
			if (i != strlen(dirp->d_name)) continue;
			strcat(linetmp, dirp->d_name);	
		}
		else continue;

		Gf_writeline(fd, linetmp, strlen(linetmp));

		memset(pathtmp, 0, GF_MAX_PATH_LEN);
		memset(&buf, 0, sizeof(buf));
		memset(linetmp, 0, GF_MAX_LINE_LEN);
	}
	closedir(predir);
	Gf_writeline(fd, linetmp, 0);
	return 0;
}

int unknown(int fd, gf_request_t* request)
{
	char linetmp[GF_MAX_LINE_LEN]= {'\0'};
	Gf_writeline(fd, "RESPOND GFRESETYPE", 18);
	Gf_writeline(fd, linetmp, 0);
	return 0;
}
