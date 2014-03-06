/*
 * only1.h - definitions for only1.c
 */

#define LOCKFILE "/tmp/gfserver.pid"

/* 
 * lock the file refered to by fd,
 * return 0 on success, or -1 reutrned
 * and errno is set on error
 */
int lock_file(int fd);

/*
 * no instance is running, return 0
 * or other values when there's one instance running
 */
int already_running(const char *filename);
