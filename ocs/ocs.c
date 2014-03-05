/*
 * ocs.c 
 */

#include "ocs.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


OCS_PORT = 9999;  // ocs listening port

int main()
{
    struct sockaddr_in ocsaddr;
    int ocsfd, dcsfd;
    int on = 1;

    ocsfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == ocsfd ) {
        perror("socket");
    }
    setsockopt(ocsfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&ocsaddr, 0, sizeof(ocsaddr));
    ocsaddr.sin_family = AF_INET;
    ocsaddr.sin_port = htons(OCS_PORT);
    ocsaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(ocsfd, (struct sockaddr *)&ocsaddr, sizeof(ocsaddr));
    listen(ocsfd, 10);

    while (1) {
        int connfd, socklen;
        struct sockaddr_in dcsaddr;
        char input_buf[20] = {'\0',};
        char *rcvmsg_buf, sndmsg_buf;
        socklen = sizeof(dcsaddr);

        connfd = accept(ocsfd, (struct sockaddr *)&dcsaddr, (socklen_t *)&socklen);
        if (-1 == connfd) {
            perror("accept");
            continue;
        }

        printf("A Client Has Connected In!\n IP: %s, port: %d\n", \
            inet_ntoa(dcsaddr.sin_addr), ntohs(dcsaddr.sin_port));

        while (1) {
            printf("Your Command: ");
            gets(input_buf);
            if (strncmp(input_buf, "close ocs", strlen("close ocs")) == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
                printf("OCS will bi closing...\n");
                sleep(2);
                exit(0);
            }
            if (strncmp(input_buf, "close current connection") == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
                printf("Close Current Connection...\n");
                sleep(2);
                close(connfd);
                break;
            }
            if (strncmp(input_buf, "dcs_tran", strlen("dcs_tran")) == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
                
            }
            if (strncmp(input_buf, "") == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
            }
            if (strncmp(input_buf, "") == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
            }
            if (strncmp(input_buf, "") == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
            }
            if (strncmp(input_buf, "") == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
            }
            if (strncmp(input_buf, "") == 0) {
                memset(input_buf, '\0', sizeof(input_buf));
            }
        }
    }

    return 0;
}
