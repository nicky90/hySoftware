/*
 * gfserver.c - main thread
 */

#include "gfserver.h"
#include "../common/gferror.h"
#include "gfconfig.h"
#include "../common/gfsignal.h"
#include "../common/gfsock.h"
#include "../common/only1.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#define	GF_SERVCONF_NAME "serverconf"
#define GF_SATCONF_NAME  "satconf"

static int listenfd;
gf_server_conf_t    gf_server_conf;
gf_server_satconf_t *gf_server_satconf;


int gf_init_server()
{
	int i;
	struct sockaddr_in servaddr;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		GF_ERROR(GFSOCKCREATERR);  /* socket creat error */

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(gf_server_conf.port);

	i = 1;
	if (0 != setsockopt(listenfd, \
			SOL_SOCKET, \
			SO_REUSEADDR, \
			&i, sizeof(int))
	) GF_ERROR(GFSOCKSETREUSEERR);  /* set socket reuse error */
		
	if(0 != bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))	
		GF_ERROR(GFSOCKBINDERR);  /* socket bind error */

	if (0 != listen(listenfd, gf_server_conf.max_thread*2))	
		GF_ERROR(GFSOCKLISTENERR);  /* socket listen error */
	for(i = 1; i <= gf_server_conf.max_thread; i++)
		/* the read creat error */
		if (0 != gf_make_thread(i)) GF_ERROR(GFTHREADCREATERR);

	return 0;
}

int gf_make_thread(int id)
{
	pthread_t pt;
	
	return pthread_create(&pt, NULL, gf_req_thread, (void*)&id);
}

void* gf_req_thread(void *id)
{
	int readbytes;
	int connfd;
	char reqline[GF_MAX_REQ_LINE_LEN] = {'\0'};
	char *p;
	Handler theHandler;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	gf_request_t gf_request;
	memset(&gf_request, 0, sizeof(gf_request_t));

	//printf("Thread %d start\n", (int)id);
	for(;;)
	{
		if ((connfd = gf_accept_safe(listenfd, \
				(struct sockaddr*)&cliaddr, \
				&clilen)) < 0
		) GF_PRINTERR(errno);
		if (0 != gf_set_read_timeout(connfd, gf_server_conf.timeout)) 
			GF_PRINTERR(errno);
		if (0 != gf_set_write_timeout(connfd, gf_server_conf.timeout)) 
			GF_PRINTERR(errno);

		/***************** get request***************************/
		if (gf_readline(connfd, reqline, GF_MAX_REQ_LINE_LEN) < 0)
			GF_PRINTERR(errno);
		if (0 == strncmp(reqline,"GET",3))
			strncpy(gf_request.type, "GET", 3);
		else if (0 == strncmp(reqline, "LIST", 4))
			strncpy(gf_request.type, "LIST", 4);
		else 
			strncpy(gf_request.type, "UNKNOWN", 7);

		//printf("thread[%d]read[%s]\n",(int)id,reqline);
		memset(reqline, 0, GF_MAX_REQ_LINE_LEN);	
		while((readbytes = gf_readline(connfd, reqline, GF_MAX_REQ_LINE_LEN)) != 0)
		{
			if (readbytes < 0)
			{
				GF_PRINTERR(errno);
				break;
			}
			else if (0 == strncmp(reqline, "satelite", 8)) {
				p = reqline + 10;
				strcpy(gf_request.satelite, p);	
			} 
			else if (0 == strncmp(reqline, "orbit", 5)) {
				p = reqline + 7;
				strcpy(gf_request.orbit, p);	
			}
			else if (0 == strncmp(reqline, "channel", 7)) {
				p = reqline + 9;
				strcpy(gf_request.channel, p);	
			}
			else if (0 == strncmp(reqline, "file", 4)) {
				p = reqline + 6;
				strcpy(gf_request.file, p);	
			}
			else if (0 == strncmp(reqline, "level", 5)) {
				p = reqline + 7;
				strcpy(gf_request.level, p);	
			}
			else if (0 == strncmp(reqline, "offset", 6)) {
				p = reqline + 8;
                gf_request.offset = strtoll(p, NULL, 10);
				/* sscanf(p, "%lld", &gf_request.offset); */
			}
			else if (0 == strncmp(reqline, "size", 4)) {
				p = reqline + 6;
                gf_request.size = strtoull(p, NULL, 10);
				/* sscanf(p, "%lld", &gf_request.size); */
			}
			else if (0 == strncmp(reqline, "check", 5)) {
				p = reqline + 7;
				sscanf(p, "%d", &gf_request.check);
			}
			else if (0 == strncmp(reqline, "compress", 8)) {
				p = reqline + 10;
				sscanf(p, "%d", &gf_request.compress);
			}
			else printf("wrong seg\n");
			//printf("thread[%d]readThenExe[%d]bytes[%s]\n", (int)id, readbytes, reqline);
			memset(reqline, 0, GF_MAX_REQ_LINE_LEN);	
		}

		if(readbytes<0)
		{
			printf("client is closed\n");
			close(connfd);
			continue;
		}
	

	/***************get handler and execute********************/
		if(NULL != (theHandler = gf_lookup_handler(gf_request.type)))
			theHandler(connfd, &gf_request);
		else printf("wrong msg type!!!\n");

		shutdown(connfd, SHUT_RDWR);
		close(connfd);
		memset(&gf_request, 0, sizeof(gf_request));
	
	//	printf("head over\n");
	}
	 	
	return 0;
}

Handler gf_lookup_handler(char *req_type)
{
	int i=0;
	while(gf_req_handler_table[i].handler != NULL ) {
		if(0 == strcmp(req_type, gf_req_handler_table[i].type))
			return gf_req_handler_table[i].handler;
			i++;
	}

	return NULL;
}



int main()
{
	gf_server_satconf_t *gf_server_tmp;

    if (already_running(LOCKFILE))
        return 1;

	if (gf_init_server_config(GF_SERVCONF_NAME, &gf_server_conf) < 0)
		GF_ERROR(errno);
	if (gf_init_server_satellite(GF_SATCONF_NAME, &gf_server_satconf) < 0)
		GF_ERROR(errno);
	gf_server_tmp=gf_server_satconf;

	printf("==========satellite config==========\n");
	while(gf_server_tmp != NULL)
	{
		printf("name = %s\n", gf_server_tmp->name);
		printf("root = %s\n", gf_server_tmp->root);
		printf("check = %d\n",gf_server_tmp->check);
		printf("compress = %d\n", gf_server_tmp->compress);
			gf_server_tmp=gf_server_tmp->next;
	}

	if (gf_init_signal() < 0)
		GF_ERROR(errno);
	//gf_server_conf.port = 5678;
	//gf_server_conf.max_thread = 2;
	//gf_server_conf.timeout = 100;
	if (0 != gf_init_server())
		GF_ERROR(errno);

	for (;;) pause();

	return 0;
}
