/*
 * gftmcmd.c
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "gftmcmd.h"
#include "gfcompress.h"
#include "gfchecksum.h"
#include "gfsock.h"
#include "only1.h"
#include "gftmworkqueue.h"

/*------------better read these from config file------------*/
#define SVR_PORT 	4746          // MDJ listen PORT
#define SVR_IP		"127.0.0.1"   // MDJ SERVER IP
#define MAX_RECONN	4             // reconnect times

#define UI_PORT		4747          // BJ listen PORT
/*----------------------------------------------------------*/

#define UI_MSG_SIZE  sizeof(ui_msg_t)
#define REQ_MSG_SIZE 1024
#define RESPONSE_BUF 1024

int main()
{
	struct sockaddr_in ui_addr;
	int ui_listfd;
	int on = 1;

	int connfd, socklen;
	struct sockaddr_in ui_connaddr;
	ui_msg_t *buf;

	// make sure only one instance running
	if (already_running(LOCKFILE))
		return 0;

	ui_listfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(ui_listfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&ui_addr, 0, sizeof(ui_addr));
	ui_addr.sin_family = AF_INET;
	ui_addr.sin_port = htons(UI_PORT);
	ui_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(ui_listfd, (struct sockaddr *)&ui_addr, sizeof(ui_addr));
	listen(ui_listfd, 10);

	/* give the control to task manager */
    if (gf_tm_init("GFGroupDB", "GFGroup", "13910821754!!")) exit(-1);

	/* then accept ui's command/request and respond*/
	for (;;)
	{
		socklen = sizeof(struct sockaddr_in);
		connfd = accept(ui_listfd, \
				(struct sockaddr *)&ui_connaddr, &socklen);
		if (connfd == -1) {
			perror("accept");
			continue;
		}
		buf = (ui_msg_t *)malloc(UI_MSG_SIZE);
		memset(buf, '\0', UI_MSG_SIZE);
		if (read(connfd, buf, UI_MSG_SIZE) == UI_MSG_SIZE) {
				// remember free buf in each switch
			if (!strncmp(buf->req_name, "pause", 5)) {
				// validate the args
				if ((strncmp(buf->req_args[0], "GF01", 4)&&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, \
						"UI_PAUSE: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_pause_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
				//sleep(1);
			}
			else if (!strncmp(buf->req_name, "resume", 6)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, \
						"UI_RESUME: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_resume_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "cancel", 6)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, \
						"UI_CANCEL: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_cancel_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "orbretran", 10)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, \
						"UI_ORBRETRAN: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_retran_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "chnretran", 9)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0)
				) {
					fprintf(stderr, \
						"UI_CHNRETRAN: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_retran_chn(buf->req_args[0], \
						buf->req_args[1], \
						buf->req_args[2]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "fileretran", 10)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0) || \
					(strlen(buf->req_args[3]) == 0) 
				) {
					fprintf(stderr, \
						"UI_FILERETRAN: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_retran_file(buf->req_args[0], \
						buf->req_args[1], \
						buf->req_args[2], \
						buf->req_args[3]);
				free(buf);
			}
#if 0
			else if (!strncmp(buf->req_name, "addtask", 6)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, \
						"UI_ADDTASK: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_add_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "addchn", 6)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0)
				) {
					fprintf(stderr, \
						"UI_ADDCHN: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_add_chn(buf->req_args[0], \
					buf->req_args[1], buf->req_args[2]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "addfile", 7)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0) || \
					(strlen(buf->req_args[3]) == 0) 
				) {
					fprintf(stderr, \
						"UI_ADDFILE: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_tm_add_file(buf->req_args[0], \
					buf->req_args[1], \
					buf->req_args[2], buf->req_args[3], buf->req_args[4]);
				free(buf);
			}
#endif
			else if (!strncmp(buf->req_name, "restart", 7)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "bj", 2) &&
					strncmp(buf->req_args[0], "md", 2) &&
					strncmp(buf->req_args[0], "both", 4)) 
				) {
					fprintf(stderr, \
						"UI_RESTART: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_restart(buf->req_args[0]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "shutdown", 8)) {
				// validate the args
				if (/* invalid args */ 
					(strncmp(buf->req_args[0], "bj", 2) &&
					strncmp(buf->req_args[0], "md", 2) &&
					strncmp(buf->req_args[0], "both", 4)) 
				) {
					fprintf(stderr, \
						"UI_SHUTDOWN: \
						wrong arguments");
					free(buf);
					continue;
				}
				gf_shutdown(buf->req_args[0]);
				free(buf);
			}
			else {
				printf("unknown request from ui\n");
				free(buf);
				continue;
			}
		}
		else {
			printf("receive msg failed\n");
			free(buf);
		}
	}
    gf_tm_destroy();

	exit(EXIT_SUCCESS);
}

#if 0
static char request_buf[REQ_MSG_SIZE];
static char response_buf[RESPONSE_BUF];

int connect_server()
{
	int try_times = 0;
	int on = 1;
	int exit_loop = 0;
	int sockfd;
	struct sockaddr_in servaddr;

	do{
		try_times++;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SVR_PORT);
		inet_pton(AF_INET, SVR_IP, &servaddr.sin_addr);

		if (connect(sockfd, (struct sockaddr*)&servaddr, \
				sizeof(servaddr)) != 0) {
			if (try_times == MAX_RECONN) {
				fprintf(stderr, "Can't connect Server, \
					Check the Network!\n");
				close(sockfd);
				exit(EXIT_FAILURE); // define ERRCONN  
			}
			fprintf(stderr, "the %dth time connection failed. \
				Try again...\n", try_times);
			sleep(3);
		}
		else {
			try_times = 0;
			exit_loop = 1;
		}
	} while (!exit_loop);

printf("Connect Success. socket fd = %d\n", sockfd);

	return 0;
}

int cli_request(int svrfd, gf_request_t *req)
{
/*	gf_writen(svrfd, req, sizeof(gf_request_t));  */

	return 0;
}

static int is_right_sat(const char *sat)
{
	if (strlen(sat) != 4 || \
		(strcmp(sat, "GF01") &&
		strcmp(sat, "GF02") &&
		strcmp(sat, "GF03"))
	)
		return 0;

	return 1;
}

static int is_right_orb(const char *orbnum)
{
	int i;

	if (strlen(orbnum) != 6) {
		return 0;
	}
	for (i = 0; i < 6; i++)
		if (!isdigit((int)orbnum[i]))
			return 0;

	return 1;
}

static int is_right_chn(const char *chn)
{
	if (strlen(chn) != 2 || \
		(strcmp(chn, "01") && strcmp(chn, "02"))
	)
		return 0;

	return 1;
}
int gf_tm_get_block(char *buf, const char *sat, const char *orb, \
		const char *chn, const char *filename, \
		int64_t offset, int64_t size, int check, int compress)
{
	int sockfd;
	int try_times = 0;
	int on = 1;
	int exit_loop = 0;
	struct sockaddr_in servaddr;

	char offset_buf[20] = {'\0'};
	char size_buf[20] = {'\0'};
	char check_buf[20] = {'\0'};
	char compress_buf[20] = {'\0'};

	do{
		try_times++;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SVR_PORT);
		inet_pton(AF_INET, SVR_IP, &servaddr.sin_addr);

		if (connect(sockfd, (struct sockaddr*)&servaddr, \
				sizeof(servaddr)) != 0) {
			if (try_times == MAX_RECONN) {
				fprintf(stderr, "Can't connect Server, \
					Check the Network!\n");
				close(sockfd);
				exit(EXIT_FAILURE); // define ERRCONN  
			}
			fprintf(stderr, "the %dth time connection failed. \
				Try again...\n", try_times);
			sleep(3);
		}
		else {
			try_times = 0;
			exit_loop = 1;
		}
	} while (!exit_loop);

printf("Connect Success. socket fd = %d\n", sockfd);

	memset(request_buf, '\0', REQ_MSG_SIZE);
	strcat(request_buf, "GET\n");
	strcat(request_buf, sat);
	strcat(request_buf, "\n");
	strcat(request_buf, orb);
	strcat(request_buf, "\n");
	strcat(request_buf, chn);
	strcat(request_buf, "\n");
	strcat(request_buf, filename);
	strcat(request_buf, "\n");
	sprintf(offset_buf, "%d", offset);
	strcat(request_buf, offset_buf);
	strcat(request_buf, "\n");
	sprintf(size_buf, "%d", size);
	strcat(request_buf, size_buf);
	strcat(request_buf, "\n");
	sprintf(check_buf, "%d", check);
	strcat(request_buf, check_buf);
	strcat(request_buf, "\n");
	sprintf(compress_buf, "%d", compress);
	strcat(request_buf, compress_buf);
	strcat(request_buf, "\n");
	strcat(request_buf, "\n");

/*	gf_writen(sockfd, request_buf, REQ_MSG_SIZE); */
/*	while (gf_readline(sockfd, response_buf, RESPONSE_BUF)); */
}

#endif

