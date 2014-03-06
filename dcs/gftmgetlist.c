/*
 * gftmdata.c - GF data interface
 *
 * Writen by CX3201 2013-07-04
 */

#include "gftmdata.h"
#include "../common/gferror.h"
#include "../common/gfsock.h"
#include "../common/gfcompress.h"
#include "gftmconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#define GET 1314
#define LIST 2013
#define MAX_RECONN 3

int tm_connect_server(char* sat)
{
        int try_times = 0;
        int on = 1;
        int exit_loop = 0;
	int sockfd = -1;
        struct sockaddr_in servaddr;
	gf_tm_sat_config_t *presatconf;
	presatconf = pgf_tm_sat_config;
	while(presatconf != NULL && strcmp(sat, presatconf->name) != 0)
		presatconf = presatconf->next;
	if(presatconf == NULL) return -1;
        /* connect server until connected or 3 times reached */
        do{
                try_times++;
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

                memset(&servaddr, 0, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(presatconf->port);
                memcpy(&servaddr.sin_addr, &presatconf->server, sizeof(struct in_addr));

                if (connect(sockfd, (struct sockaddr*)&servaddr, \
                                sizeof(servaddr)) != 0) {
			close(sockfd);
                        if (try_times == MAX_RECONN) {
                                fprintf(stderr, "Can't connect Server, \
                                        Check the Network!\n");
				perror("connect");
				GF_ERROR(GFCONNERR);
                        }
                        fprintf(stderr, "the %dth time connection failed.\
                                Try again...\n", try_times);
			perror("connect");
                        sleep(3);
                }
                else {
                        try_times = 0;
                        exit_loop = 1;
                }
        } while (!exit_loop);
        /* set socket read and write timeout */
        if (0 != gf_set_write_timeout(sockfd, gf_tm_config.timeout))
	{
		close(sockfd);
		GF_ERROR(errno);
	}
	if (0 != gf_set_read_timeout(sockfd, gf_tm_config.timeout))
	{
		close(sockfd);
		GF_ERROR(errno);
	}
printf("Connect Success. socket fd = %d\n", sockfd);

        return sockfd;
}

int gf_tm_get_block(uint8_t *buf, char *sat, char *orb, char *chn, char *filename, int64_t offset, int64_t size, int check, int compress)
{
	int skfd;
	char linetmp[1024];
	if((skfd=tm_connect_server(sat)) <= 0) GF_ERROR(errno);
/***********REQUEST*****WAY1****************************/

	gf_writeline(skfd, "GET", 3);
        memset(linetmp,0,1024);
        sprintf(linetmp, "satelite: %s", sat);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "orbit: %s", orb);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "channel: %s", chn);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "offset: %lld", (long long int)offset);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "size: %lld", (long long int)size);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "file: %s", filename);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "check: %d", check);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        memset(linetmp,0,1024);
        sprintf(linetmp, "compress: %d", compress);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        gf_writeline(skfd, linetmp, 0);

/********REQUEST********WAY2*****************************************/
/*
        memset(linetmp,0,1024);
	sprintf(linetmp,"GET\n"\
			"satelite: %s\norbit: %s\nchannel: %s\n"\
			"offset: %lld\nsize: %lld\nfile: %s\n"\
			"check: %d\ncompress: %d\n", \
			 sat, orb, chn, \
			 offset, size, filename, \
			 check, compress);
        gf_writeline(skfd, linetmp, strlen(linetmp));
*/

/********GET***HEAD***************************************************/
	int flag=-1,comflag=0,checkflag=0,readbyte=0;
	char Check16[20], Check32[40],*zmsg,getmsg[1024];
	char *p;
	int64_t  realsize=0, orgsize=0, zsize=0;
	uint64_t unzsize=(uint64_t)size; 
	memset(getmsg,0,1024);
//int64_t time1,time2;
//time1=time(NULL);
	while((readbyte=gf_readline(skfd, getmsg, 1024)) > 0)
		{
		
		if(strncmp(getmsg,"RESPOND", 7)==0)
			{
			p=getmsg+8;
			if(strncmp(p, "GET", 3)== 0) flag=GET;
			//else if(strncmp(p, "LIST", 4)== 0) flag=LIST;
			else if(strncmp(p, "WRONG", 5)== 0)
				{
				sscanf(p+5,"%d",&flag);
				}
			else flag = GFINVAPARAM;
			}
		if(strncmp(getmsg,"ORGSIZE", 7)==0)
			{
			p=getmsg+8;
			sscanf(p,"%lld",(long long int *)&orgsize);
			if(0 == zsize)
				realsize=orgsize;
			}
		if(strncmp(getmsg,"ZSIZE", 5)==0)
			{
			p=getmsg+6;
			sscanf(p,"%lld",(long long int *)&zsize);
			realsize=zsize;
			}
		if(strncmp(getmsg,"COMPRESS", 8)==0)
			{
			p=getmsg+9;
			sscanf(p,"%d",&comflag);
			}
		if(strncmp(getmsg,"MD5SUM", 6)==0)
			{
			p=getmsg+7;
			memcpy(Check32, p ,32);
//printf("read[%d]MDSUM[%s]in",readbyte,getmsg);
//printf("block[%d]\n",pre_offset/MSGSIZE);
			gf_md5_32to16(Check32,(uint8_t *)Check16);
			checkflag = 1;
			}
		memset(getmsg,0,1024);
		}
//time2=time(NULL);
/******MAKE*SURE**READLINE**RIGHT******************************************************************/
		if (readbyte < 0)
			{
			close(skfd);
			GF_ERROR(errno);
			}
/*****MAKE*SURE**RESPOND***RIGHT******************************************************************/
		if (flag != GET)
			{
			close(skfd);
			GF_ERROR(flag);
			}
/***********GET***DATA**********************************************************/
                if (comflag > 0)
                        {
			zmsg = (char *)malloc(size);
                	if(gf_readn(skfd,zmsg,(ssize_t)realsize) <= 0)
				{
				close(skfd);
                        	free(zmsg);
				GF_ERROR(errno);
				}
                        if(gf_uncompress_data((char *)buf, &unzsize, zmsg, (uint64_t)realsize) < 0)
				{
				close(skfd);
                        	free(zmsg);
				GF_ERROR(errno);
				}

                        if(orgsize == (int64_t)unzsize && size == orgsize)
                        	free(zmsg);
			else 	{
				free(zmsg);
				close(skfd);
				GF_ERROR(GFDATAERR);
				}

                        }
		else if(0 == comflag && gf_readn(skfd, buf, (ssize_t)realsize) <= 0)
				{
				close(skfd);
				GF_ERROR(errno);
				}	
//while(0 < gf_readline(skfd, getmsg, 1024))     {continue;}
		close(skfd);
                if( checkflag > 0 &&0 != gf_checkcmp_data(Check16, (char*)buf, (int)size))
                        {
                        printf("Check Not OK!\n");
//pwrite(fifd2,unzmsg,blocksize,pre_offset);
			GF_ERROR(errno); 
                       	}
               // else printf("data get OK!!!\n");
//pwrite(fifd2,oridata,blocksize,pre_offset);
			
		return 0;
}


int gf_tm_poll_sat(char *sat, int (*fn)(char *sat, char *orb))
{
	int skfd,flag=0,readbyte=0;
	char *p;
	char linetmp[1024],getmsg[1024];
	if((skfd=tm_connect_server(sat)) <= 0) GF_ERROR(GFCONNERR);
	gf_writeline(skfd, "LIST", 4);
        gf_writeline(skfd, "level: orb", 10);
        memset(linetmp,0,1024);
        sprintf(linetmp, "satelite: %s", sat);
        gf_writeline(skfd, linetmp, strlen(linetmp));
        gf_writeline(skfd, linetmp, 0);
    	while((readbyte=gf_readline(skfd, getmsg, 1024)) > 0)
	{
		if(strncmp(getmsg,"RESPOND", 7)==0)
			{
			p=getmsg+8;
			//if(strncmp(p, "GET", 3)== 0) flag=GET;
			if(strncmp(p, "LIST", 4)== 0) flag=LIST;
			else if(strncmp(p, "WRONG", 5)== 0)
				{
				sscanf(p+5,"%d",&flag);
				printf("msg2[%s]\n",getmsg);
				}
			else flag = GFINVAPARAM;
			}
	memset(getmsg,0,1024);
	}
	
/******MAKE*SURE**READLINE**RIGHT******************************************************************/
	if (readbyte < 0)
		{
		close(skfd);
		GF_ERROR(errno);
		}
/*****MAKE*SURE**RESPOND***RIGHT******************************************************************/
	if(flag != LIST)
		{
		close(skfd);
		GF_ERROR(flag);
		}
	while(gf_readline(skfd, getmsg, 1024 ) > 0)
		{	
		if(fn(sat, getmsg)) GF_ERROR(errno);
		memset(getmsg,0,1024);
		}
	close(skfd);
	return 0;
}

int gf_tm_poll_task(char *sat, char *orb, int (*fn)(char *sat, char *orb, char *chn))
{
	int skfd,flag=0,readbyte;
	char *p;
	char linetmp[1024],getmsg[1024];
	if((skfd=tm_connect_server(sat)) <= 0) GF_ERROR(GFCONNERR);
	gf_writeline(skfd, "LIST", 4);
        gf_writeline(skfd, "level: chn", 10);

        memset(linetmp,0,1024);
        sprintf(linetmp, "satelite: %s", sat);
        gf_writeline(skfd, linetmp, strlen(linetmp));

        memset(linetmp,0,1024);
        sprintf(linetmp, "orbit: %s", orb);
        gf_writeline(skfd, linetmp, strlen(linetmp));

        gf_writeline(skfd, linetmp, 0);
    	while((readbyte=gf_readline(skfd, getmsg, 1024)) > 0)
	{
		if(strncmp(getmsg,"RESPOND", 7)==0)
			{
			p=getmsg+8;
			if(strncmp(p, "LIST", 4)== 0) flag=LIST;
			else if(strncmp(p, "WRONG", 5)== 0)
				{
				sscanf(p+5,"%d",&flag);
				printf("msg3[%s]\n",getmsg);
				}
			else flag = GFINVAPARAM;
			}
	memset(getmsg,0,1024);
	}
/******MAKE*SURE**READLINE**RIGHT******************************************************************/
	if (readbyte < 0)
		{
		close(skfd);
		GF_ERROR(errno);
		}
/*****MAKE*SURE**RESPOND***RIGHT******************************************************************/
	if(flag != LIST)
		{
		close(skfd);
		GF_ERROR(flag);
		}
	while(gf_readline(skfd, getmsg, 1024 ) > 0)
		{	
		if(fn(sat, orb, getmsg)) GF_ERROR(errno);
		memset(getmsg,0,1024);
		}
	
	close(skfd);
	return 0;
}

int gf_tm_poll_chn(char *sat, char *orb, char *chn, int (*fn)(char *sat, char *orb, char *chn, char *filename, int64_t size))
{
	int skfd,flag=0,readbyte,finflag=0,filecount=0;
	int64_t prefilesize=0;
	char *p;
	char linetmp[1024],getmsg[1024],prefile[1024];
	if((skfd=tm_connect_server(sat)) <= 0) GF_ERROR(GFCONNERR);
	gf_writeline(skfd, "LIST", 4);
        gf_writeline(skfd, "level: file", 11);

        memset(linetmp,0,1024);
        sprintf(linetmp, "satelite: %s", sat);
        gf_writeline(skfd, linetmp, strlen(linetmp));

        memset(linetmp,0,1024);
        sprintf(linetmp, "orbit: %s", orb);
        gf_writeline(skfd, linetmp, strlen(linetmp));

        memset(linetmp,0,1024);
        sprintf(linetmp, "channel: %s", chn);
        gf_writeline(skfd, linetmp, strlen(linetmp));

        gf_writeline(skfd, linetmp, 0);
    	while((readbyte=gf_readline(skfd, getmsg, 1024)) > 0)
	{
		if(strncmp(getmsg,"RESPOND", 7)==0)
			{
			p=getmsg+8;
			//if(strncmp(p, "GET", 3)== 0) flag=GET;
			if(strncmp(p, "LIST", 4)== 0) flag=LIST;
			else if(strncmp(p, "WRONG", 5)== 0)
				{
				sscanf(p+5,"%d",&flag);
				printf("msg4[%s]\n",getmsg);
				}
			else flag = GFINVAPARAM;
			}
	memset(getmsg,0,1024);
	}
/******MAKE*SURE**READLINE**RIGHT******************************************************************/
	if (readbyte < 0)
		{
		close(skfd);
		GF_ERROR(errno);
		}
/*****MAKE*SURE**RESPOND***RIGHT******************************************************************/
	if(flag != LIST)
		{
		close(skfd);
		printf("flag=%d\n",flag);
		GF_ERROR(flag);
		}
	while(gf_readline(skfd, getmsg, 1024 ) > 0)
		{
		p=getmsg;	
		if(strncmp(getmsg, "finish", 6) == 0)
			sscanf(p+6,"%d",&finflag);
		else 
			{
			sscanf(p,"%lld %s",(long long int *)&prefilesize, prefile);
			if(fn(sat, orb, chn, prefile, prefilesize)) GF_ERROR(errno);
			filecount++;
			}
		memset(getmsg,0,1024);
		memset(prefile,0,1024);
		}
	//if(filecount != finflag) finflag=0;
	if(filecount > 0) finflag=filecount;
	
	close(skfd);
	return finflag;
}

