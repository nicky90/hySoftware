/*
 * only1.c - make sure there is only one instance running
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "only1.h"

#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int lock_file(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	/* lock the whole file */
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	return (fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		printf("Can't open %s: %m\n", filename);
		exit(EXIT_FAILURE);
	}

	if (lock_file(fd) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			read(fd, buf, 20);
			printf("another Instance already running, "\
					"pid = %s\n", buf);
			close(fd);
			exit(EXIT_FAILURE);
		}
		printf("Can't lock %s: %m\n", filename);
		exit(EXIT_FAILURE);
	}

	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);

	return 0;
}

#if 0
/* " how to use " */
int main()
{
	if (already_running(LOCKFILE))
		return 0;

/**********  PUT YOUR CODE HERE **********/
	printf("main start\n");
	sleep(100);
	printf("main done\n");
/*****************************************/

	exit(EXIT_SUCCESS);
}
#endif 
