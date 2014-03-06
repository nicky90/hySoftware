#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "../common/gfcompress.h"
#include "../common/gfchecksum.h"
#include "../common/gfsock.h"
#include "gftmgetlist.c"
#include "gftmconfig.c"

#define MSGSIZE (1<<22)
#define STDIN 0
#define GET 32
#define LIST 34
#define Sat "GF01"
#define Orb "1231256"
#define Chn "01"
#define Check 1
#define Compress 1

int serv_port=5678;
int msgsize=MSGSIZE;
uint64_t filesize=0;
char filepath[1024],destpath[1024],filename[1024];
static uint64_t offset=0;
static pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
void* BlockThread(void *id)
{
	int fifd2;
	char sendmsg[1024],*oridata,*unzmsg;
	char *buf;
	char tmp[10],seg[10],linetmp[1024];
	int64_t blocksize=0,pre_offset=0;
	//printf("thread start[%d]\n",(int)id);
   while(1){
	pthread_mutex_lock(&mlock);
	pre_offset=offset;
	if(filesize - offset < MSGSIZE) {blocksize=filesize-offset;offset=filesize;}
	else {blocksize=MSGSIZE;offset+=MSGSIZE;}
	pthread_mutex_unlock(&mlock);
	if(blocksize <= 0) 
		pthread_exit(NULL);
	buf=(char *)malloc(blocksize);
	if(gf_tm_get_block((uint8_t *)buf, Sat, Orb, Chn, filename, pre_offset, blocksize, Check, Compress) !=1)
		printf("getblock[%d] wrong!",pre_offset/MSGSIZE);
	fifd2 = open(destpath, O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	pwrite(fifd2,buf,blocksize,pre_offset);
	free(buf);
	close(fifd2);
	}
}
int printChn(char *sat, char* orb, char* chn, char *file, int64_t size)
	{
	printf("sat[%s]:\torb[%s]:\tchn[%s]\tfile[%s:%lld]\n", sat, orb, chn, file, size);
	return 0;
	}

int printOrb(char *sat, char* orb, char* chn)
	{
	int i;
	printf("sat[%s]:\torb[%s]:\tchn[%s]\n", sat, orb, chn);
	if((i=gf_tm_poll_chn(sat, orb, chn, printChn))>0);
		printf("finished[%d]",i);
	printf("^^^^^^^^^^^^^^^^^^^^^\n");
	return 0;
	}

int printSat(char *sat, char* orb)
	{
	printf("sat[%s]:\torb[%s]\n", sat, orb);
	gf_tm_poll_task(sat,orb,printOrb);
	printf("*****************\n");
	return 0;
	}

int Getfilesize(char *sat, char* orb, char* chn, char *file, int64_t size)
	{
	if(strcmp(file,filename)==0) 
		{
		printf("file[%s]size[%lld]\n",file, size);
		filesize=size;
		return 0;
		}
	else {
		//printf("File[%s]size[%lld]\n",file, size);
		return 0;}
	}
int main(int argc, char *argv[]){
	if(argc != 2 )
		{
		printf("wrong segment!\n");
		return -1;
		}
	void *val;
	sprintf(filepath,"/data/%s/%s/%s/%s", Sat, Orb, Chn, argv[1]);
	sprintf(filename,"%s",argv[1]);
	sprintf(destpath,"/data/%s",argv[1]);
	
	if(gf_tm_read_config("tm.cfg")) GF_ERROR_PR(errno);
        if(gf_tm_read_sat_config("tmsat.cfg")) GF_ERROR_PR(errno);
	gf_tm_poll_sat(Sat,printSat);	
	gf_tm_poll_chn(Sat,Orb,Chn,Getfilesize);
	pthread_t pt[32];
	int i=0;
	for(i=0; i < 8; i++)
		pthread_create(&pt[i], NULL, BlockThread, (void*)i);
	for(i=0; i <8 ; i++)
		pthread_join(pt[i],&val);
	return 0;
}
