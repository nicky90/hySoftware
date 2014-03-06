/*
 * gfsock.h - socket function wrapper
 *
 * Writen by CX3201 2013-05-29
 */

#ifndef __GFSOCK_H__
#define __GFSOCK_H__

#include <sys/types.h>
#include <sys/socket.h>

/* Accept with mutex */
int gf_accept_safe(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/* Read n bytes */
ssize_t gf_readn(int fd, void *vptr, size_t n);
/* Write n bytes */
ssize_t gf_writen(int fd, void *vptr, size_t n);
/* Read a line, read a string end with "\n" and delete the "\n" */
ssize_t gf_readline(int fd, void *vptr, size_t nbuf);
/* Read a line, add a "\n" to the end of string and write to fd */
ssize_t gf_writeline(int fd, void *vptr, size_t nlen);

/* Set read timeout to fd, timeo in seconds */
int gf_set_read_timeout(int fd, int timeo);
/* Set write timeout to fd, timeo in seconds */
int gf_set_write_timeout(int fd, int timeo);

#endif /* __GFSOCK_H__ */
