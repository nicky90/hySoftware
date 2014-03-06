/*
 * gftm.h - GF task manager work queue definition
 *
 * Writen by CX3201 2013-06-20
 */

#ifndef __GFTMWORKQUEUE_H__
#define __GFTMWORKQUEUE_H__

#include <stdint.h>
#include "gftm.h"

#define GF_TM_WORK_ABORT 1
#define GF_TM_WORK_EXIT  2

#define GF_TM_BLOCK_FREE  0
#define GF_TM_BLOCK_DONE  1
#define GF_TM_BLOCK_ERROR 2

#define GF_TM_TRANS_QUEUE 1
#define GF_TM_PAUSE_QUEUE 2

typedef struct __gf_tm_workspace_t{
	gf_tm_file_t file;
	char         datapath[GF_TM_PATH_MAX+1];
	char         statuspath[GF_TM_PATH_MAX+1];
	int          datafd;
	int          statusfd;
	int64_t      block_size;
	int64_t      block_count;
	int64_t      alloc_index;
	int64_t      done_count;
	int64_t      doing_count;
	int64_t      error_count;
	uint8_t     *block_state;
	int          check;
	int          compress;
	struct __gf_tm_workspace_t *next;
} gf_tm_workspace_t;

typedef struct __gf_tm_work_t{
	sem_t              data_sem;
	pthread_t          work_thread;
	int                state;
	gf_tm_workspace_t *workspace;
	int                dup_data_fd;
	gf_tm_file_t       file;
	int64_t            block_index;
	int64_t            offset;
	int64_t            size;
	int                check;
	int                compress;
	uint8_t           *buffer;
} gf_tm_work_t;

int gf_tm_init_workspace_queue(void);
void gf_tm_destroy_workspace_queue(void); /* Call before gf_tm_destroy_work_slot */
int gf_tm_alloc_work_slot(void);
void gf_tm_destroy_work_slot(void);
int gf_tm_make_work_threads();
void gf_tm_destroy_work_threads();

int gf_tm_data_broadcast(void);
int gf_tm_data_wait(int id);
int gf_tm_data_post(int id);

int gf_tm_add_task(char *sat, char *orb);
int gf_tm_add_chn(char *sat, char *orb, char *chn);
int gf_tm_add_file(char *sat, char *orb, char *chn, char *filename, int64_t size);

int gf_tm_pause_task(char *sat, char *orb);
int gf_tm_resume_task(char *sat, char *orb);

int gf_tm_cancel_task(char *sat, char *orb);

int gf_tm_retran_task(char *sat, char *orb);
int gf_tm_retran_chn(char *sat, char *orb, char *chn);
int gf_tm_retran_file(char *sat, char *orb, char *chn, char *filename);

int gf_tm_clear_workfile(char *sat, char *orb, char *chn, char *filename, char *suffix);

/*********DEBUG***********/
void gf_tm_print_workspace(gf_tm_workspace_t *p, int showblock);
void gf_tm_print_workspace_queue(void);
void gf_tm_print_workslot(gf_tm_work_t *p);
void gf_tm_print_workslots(void);

#endif /* __GFTMWORKQUEUE_H__ */

