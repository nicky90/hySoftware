/*
 * gfsock.c - safe write and read routines
 */


#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/time.h>

#include "gfsock.h"
#include "gferror.h"

static pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;


/* Accept with mutex */
int gf_accept_safe(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int connfd;

	pthread_mutex_lock(&mlock);
	connfd = accept(sockfd, addr, addrlen);
	pthread_mutex_unlock(&mlock);

	return connfd;
}

/* Read n bytes */
ssize_t	readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0; /* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;	/* EOF */

		nleft -= nread;
		ptr   += nread;
	}

	return(n - nleft);	/* return >= 0 */
}

ssize_t gf_readn(int fd, void *vptr, size_t n)
{
	ssize_t	nread;

	if ((nread = readn(fd, vptr, n)) < 0)
		{
		perror("readn");
		GF_ERROR(GFRDNERR);
		}

	return(nread);
}

/* Write n bytes */
ssize_t	writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;	/* and call write() again */
			else
				return(-1);	/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	return(n);
}

ssize_t gf_writen(int fd, void *vptr, size_t n)
{	
	ssize_t nwrite;

	if ((nwrite = writen(fd, vptr, n)) != n)
		{
		perror("writen");
		GF_ERROR(GFWRNERR);
		}

	return nwrite;
}

/* Read a line, read a string end with "\n" and delete the "\n" */
ssize_t	readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t	n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen+1; n++) {
again:
		if ( (rc = read(fd, &c, 1)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);	/* EOF, n - 1 bytes were read */
		} else {
			if (errno == EINTR)
				goto again;
			return(-1);		/* error, errno set by read() */
		}
	}
	ptr--;
	if(maxlen+1 == n && '\n'!=*ptr) return maxlen+1;
	*ptr = 0;	/* null terminate like fgets() */

	return(n);
}

ssize_t gf_readline(int fd, void *vptr, size_t nbuf)
{
	ssize_t	n;
	
	if ((n = readline(fd, vptr, nbuf)) < 0)
		{
		perror("readline");
		GF_ERROR(GFRDLINERR);
		}
	if(n > nbuf)
		{
		char tmp[nbuf],ntmp;
		ntmp = n;
		while (ntmp > nbuf)
			{if ((ntmp = readline(fd, tmp, nbuf)) < 0)
			GF_ERROR(GFRDLINERR);
			printf("readagain\n");}
			
		}
	return(n-1);
}

/* Read a line, add a "\n" to the end of string and write to fd */
ssize_t gf_writeline(int fd, void *vptr, size_t nlen)
{
	ssize_t	n;

	if((n = writen(fd, vptr, nlen)) < 0)
		{
		perror("writeline");
		GF_ERROR(GFWRLINERR);
		}
	if(writen(fd, "\n", 1) < 0)
		GF_ERROR(GFWRLINERR);
	
	return(n);
}

/* Set read timeout to fd, timeo in seconds */
int gf_set_read_timeout(int fd, int timeo)
{	
	int ret;
	struct timeval t;
	t.tv_sec=timeo;
	t.tv_usec=0;

	if((ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t))) < 0)
		GF_ERROR(GFSETRDTERR);

	return ret;
}

/* Set write timeout to fd, timeo in seconds */
int gf_set_write_timeout(int fd, int timeo)
{
	int ret;
	struct timeval t;
	t.tv_sec=timeo;
	t.tv_usec=0;

	if((ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t))) < 0)
		GF_ERROR(GFSETWRTERR);

	return ret;
}
