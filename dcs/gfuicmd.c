/*
 * gfui.c - emulate the JavaUI to shoot request/command
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define BJPORT 4747
#define BJIP "127.0.0.1"

typedef struct __ui_msg_t {
	char req_name[20];
	char req_args[5][50];
} ui_msg_t;

static void menu(void)
{
	printf("Shoot your request or command: \n");
	printf("pause <satid> <orbnum> - e.g. 'pause GF01 123456'\n");
	printf("cancel <satid> <orbnum> - e.g. 'cancel GF01 123456'\n");
	printf("resume <satid> <orbnum> - e.g. 'resume GF01 123456'\n");
	printf("correct <satid> <orbnum> - e.g. 'correct GF01 123456'\n");
	printf("orbretran <satid> <orbnum>\n");
	printf("chnretran <satid> <orbnum>\n");
	printf("fileretran <satid> <orbnum>\n");
	printf("addtask <satid> <orbnum>\n");
	printf("addchn <satid> <orbnum> <chn>\n");
	printf("addfile <satid> <orbnum> <chn> <filename>\n");
	printf("quit\n");
	printf("\n");
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

static int is_right_chn(const char *chn) {
	if (strlen(chn) != 2 || \
		(strcmp(chn, "01") && strcmp(chn, "02"))
	)
		return 0;

	return 1;
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

int main()
{
	char cmdbuffer[120];
	char cmd[20];
	char satid[20];
	char orbnum[20];
	char chn[20];
	char filename[50];

	int sockfd;
	int on = 1;
	struct sockaddr_in servaddr;
	ui_msg_t msg;

	int quit_flag = 0;

	do {
		memset(cmdbuffer, '\0', 120);
		memset(cmd, '\0', 20);
		memset(satid, '\0', 10);
		memset(orbnum, '\0', 10);
		memset(chn, '\0', 5);
		memset(filename, '\0', 45);
		memset(&msg, '\0', sizeof(msg));

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(BJPORT);
		inet_pton(AF_INET, BJIP, &servaddr.sin_addr);

		if (connect(sockfd, (struct sockaddr*)&servaddr, \
				sizeof(servaddr)) != 0) {
			perror("connect");
			sleep(1);
			close(sockfd);
		}
		else {
			menu();
			printf("cmd>> ");
			if (fgets(cmdbuffer, 121, stdin) == NULL) {
				perror("fgets");
			}
			else {
				strcpy(cmd, strtok(cmdbuffer, " "));
				if (!strcmp(cmd, "pause")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "cancel")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "resume")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "correct")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "orbretran")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "chnretran")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					strcpy(chn, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else if (!is_right_orb(chn))
						fprintf(stderr, "Wrong Chn\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						strcpy(msg.req_args[2], chn);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "fileretran")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					strcpy(chn, strtok(NULL, " "));
					strcpy(filename, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else if (!is_right_chn(chn))
						fprintf(stderr, "Wrong Chn\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						strcpy(msg.req_args[2], chn);
						strcpy(msg.req_args[3], filename);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "addtask")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "addchn")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					strcpy(chn, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else if (!is_right_chn(chn))
						fprintf(stderr, "Wrong Chn\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						strcpy(msg.req_args[2], chn);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "addfile")) {
					strcpy(satid, strtok(NULL, " "));
					strcpy(orbnum, strtok(NULL, " "));
					strcpy(chn, strtok(NULL, " "));
					if (!is_right_sat(satid))
						fprintf(stderr, "Wrong SatID\n");
					else if (!is_right_orb(orbnum))
						fprintf(stderr, "Wrong Orbnum\n");
					else if (!is_right_chn(chn))
						fprintf(stderr, "Wrong Chn\n");
					else {
						strcpy(msg.req_name, cmd);
						strcpy(msg.req_args[0], satid);
						strcpy(msg.req_args[1], orbnum);
						strcpy(msg.req_args[2], chn);
						strcpy(msg.req_args[3], filename);
						write(sockfd, &msg, sizeof(msg));
					}
				}
				else if (!strcmp(cmd, "quit")) {
					quit_flag = 1;
				}
				else {
					fprintf(stderr, "Wrong Request\n");
				}
			}
		}
	} while (!quit_flag);

	exit(EXIT_SUCCESS);
}

