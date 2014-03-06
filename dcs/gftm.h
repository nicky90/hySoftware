/*
 * gftm.h - GF task manager definition
 *
 * Writen by CX3201 2013-05-31
 */

#ifndef __GFTM_H__
#define __GFTM_H__

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "gfchecksum.h"

#define GF_TM_CONFIG_FILE     "tm.cfg"
#define GF_TM_SAT_CONFIG_FILE "tmsat.cfg"

#define GF_TM_STATUS_FILE_SUFFIX ".status"

#define GF_TM_SAT_NAME_MAX  16
#define GF_TM_ORB_NAME_MAX  16
#define GF_TM_CHN_NAME_MAX  16
#define GF_TM_FILE_NAME_MAX 128
#define GF_TM_PATH_MAX      1024
#define GF_TM_STATE_STR_MAX 8

#define GF_TM_STATE_POLL     "POLL"
#define GF_TM_STATE_TRANSMIT "TRANSMIT"
#define GF_TM_STATE_PAUSE    "PAUSE"
#define GF_TM_STATE_ERROR    "ERROR"
#define GF_TM_STATE_HANDLE   "HANDLE"
#define GF_TM_STATE_DONE     "DONE"
#define GF_TM_STATE_CANCEL   "CANCEL"
#define GF_TM_STATE_NULL     "NULL"

#define GF_TM_IS_POLL(x)     (strcmp(x, GF_TM_STATE_POLL) == 0)
#define GF_TM_IS_TRANSMIT(x) (strcmp(x, GF_TM_STATE_TRANSMIT) == 0)
#define GF_TM_IS_PAUSE(x)    (strcmp(x, GF_TM_STATE_PAUSE) == 0)
#define GF_TM_IS_ERROR(x)    (strcmp(x, GF_TM_STATE_ERROR) == 0)
#define GF_TM_IS_HANDLE(x)   (strcmp(x, GF_TM_STATE_HANDLE) == 0)
#define GF_TM_IS_CANCEL(x)   (strcmp(x, GF_TM_STATE_CANCEL) == 0)
#define GF_TM_IS_DONE(x)     (strcmp(x, GF_TM_STATE_DONE) == 0)

#define GF_TM_IS_ACTIVE(x)     (GF_TM_IS_POLL(x)||GF_TM_IS_TRANSMIT(x))
#define GF_TM_ADD_ALLOW(x)      GF_TM_IS_POLL(x)
#define GF_TM_PAUSE_ALLOW(x)   (GF_TM_IS_TRANSMIT(x)||GF_TM_IS_POLL(x))
#define GF_TM_RESUME_ALLOW(x)  (GF_TM_IS_PAUSE(x))
#define GF_TM_CORRECT_ALLOW(x)  GF_TM_IS_ERROR(x)
#define GF_TM_CANCEL_ALLOW(x)  (1)

#define GF_TM_SAME_FILE(a,b)   (strcmp(a.sat,b.sat)==0 &&\
				strcmp(a.orb,b.orb)==0 &&\
				strcmp(a.chn,b.chn)==0 &&\
				strcmp(a.filename,b.filename)==0)

#define GF_TM_TASK_PARAM(task) (task).sat, (task).orb
#define GF_TM_CHN_PARAM(chn)   (chn).sat, (chn).orb, (chn).chn
#define GF_TM_FILE_PARAM(file) (file).sat, (file).orb, (file).chn, (file).filename

/* Task desc */
typedef struct __gf_tm_task_t{
	char state[GF_TM_STATE_STR_MAX+1];
	char sat[GF_TM_SAT_NAME_MAX+1];
	char orb[GF_TM_ORB_NAME_MAX+1];
	char statebackup[GF_TM_STATE_STR_MAX+1];
	int  chnnum;
	int  retran;
} gf_tm_task_t;

/* Channel desc */
typedef struct __gf_tm_chn_t{
	char state[GF_TM_STATE_STR_MAX+1];
	char sat[GF_TM_SAT_NAME_MAX+1];
	char orb[GF_TM_ORB_NAME_MAX+1];
	char chn[GF_TM_CHN_NAME_MAX+1];
	char statebackup[GF_TM_STATE_STR_MAX+1];
	int  retran;
} gf_tm_chn_t;

/* File desc */
typedef struct __gf_tm_file_t{
	char     state[GF_TM_STATE_STR_MAX+1];
	char     sat[GF_TM_SAT_NAME_MAX+1];
	char     orb[GF_TM_ORB_NAME_MAX+1];
	char     chn[GF_TM_CHN_NAME_MAX+1];
	char     filename[GF_TM_FILE_NAME_MAX+1];
	char     statebackup[GF_TM_STATE_STR_MAX+1];
	int      priority;
	int64_t  size;
	int64_t  recv_size;
	int      retran;
} gf_tm_file_t;

#define GF_TM_HANDLE_NONE 0
#define GF_TM_HANDLE_TASK 1
#define GF_TM_HANDLE_CHN  2
#define GF_TM_HANDLE_FILE 3

#define GF_TM_FILE_HANDLER "gftmfhandler"
#define GF_TM_CHN_HANDLER  "gftmchandler"
#define GF_TM_TASK_HANDLER "gftmthandler"

typedef struct __gf_tm_handle_board_t{
	int   type;
	char  state[GF_TM_STATE_STR_MAX+1];
	char  sat[GF_TM_SAT_NAME_MAX+1];
	char  orb[GF_TM_ORB_NAME_MAX+1];
	char  chn[GF_TM_CHN_NAME_MAX+1];
	char  filename[GF_TM_FILE_NAME_MAX+1];
	pid_t pid;
} gf_tm_handle_board_t;

/* Running state */
typedef struct __gf_tm_runtime_t{
	pthread_t            statistics_thread;
	pthread_t            poll_thread;
	pthread_t            handle_thread;
	pthread_mutex_t      tm_lock;
	pthread_mutex_t      runtime_lock;
	pthread_mutex_t      handle_lock;
	sem_t                handle_sem;
	gf_tm_handle_board_t handle_board;
	int                  may_exit;
	int64_t              speed;
} gf_tm_runtime_t;

extern gf_tm_runtime_t gf_tm_runtime;

int gf_tm_lock(void);
int gf_tm_unlock(void);
int gf_tm_handle_lock(void);
int gf_tm_handle_unlock(void);
int gf_tm_handle_wait(void);
int gf_tm_handle_post(void);
int gf_tm_runtime_lock(void);
int gf_tm_runtime_unlock(void);
int gf_tm_get_speed(int64_t *pval);
int gf_tm_add_speed(int64_t val);
int gf_tm_zero_speed(void);

/*******DEBUG*********/
void gf_tm_print_file(gf_tm_file_t *pfile);

#endif /* __GFTM_H__ */
