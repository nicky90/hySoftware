/*
 * adapter.c 
 */

#include "adapter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

YK_IP = "127.0.0.1";
YK_PORT = 9999;

int main()
{
    /*connect to ocs*/
    int on = 1;
    int flag = 0;
    struct sockaddr_in sockaddr_yk;
    do {
        int yk_sock;
        if ((yk_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) continue;
        setsockopt(yk_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        memset(&sockaddr_yk, 0, sizeof(sockaddr_yk));
        sockaddr_yk.sin_family = AF_INET;
        sockaddr_yk.sin_port = htons(YK_PORT);
        inet_pton(AF_INET, YK_IP, &sockaddr_yk.sin_addr);

        if (connect(yk_sock, (struct sockaddr *)&sockaddr_yk, sizeof(sockaddr_yk)) < 0) {
            flag = 0;
        }
        else {
            flag = 1;
        }
    while (!flag);

    return 0;
}
