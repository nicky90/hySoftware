/*
 * gftmdb.h - GF task manager database
 *
 * These functions are not threads safe
 *
 * Writen by CX3201 2013-06-11
 */

#ifndef __GFTMDB_H__
#define __GFTMDB_H__

#include "gftm.h"
#include "gfdbutils.h"
#include <stdint.h>

#define GF_TM_OP_STR_MAX    32
#define GF_TM_LOGMSG_MAX    128
#define GF_TM_SQL_MAX       1024

#define GF_TM_TASK_TABLE  "gf_task"
#define GF_TM_CHN_TABLE   "gf_chn"
#define GF_TM_FILE_TABLE  "gf_file"
#define GF_TM_LOG_TABLE   "gf_log"
#define GF_TM_DEBUG_TABLE "gf_debug"
#define GF_TM_STATE_TABLE "gf_state"
#define GF_TM_TASK_INDEX  "gf_task_index"
#define GF_TM_CHN_INDEX   "gf_chn_index"
#define GF_TM_FILE_INDEX  "gf_file_index"

/* These functions are not threads safe */
int gf_tm_init_tables(void);

int gf_tm_log(char *sat, char *orbit, char *chn, char *filename, char *op, char *msg);
int gf_tm_debug(int code, const char *file, const char *func, int line, char *msg);

int gf_tm_refresh_starttime(void);
int gf_tm_refresh_state(char *state);
int gf_tm_refresh_speed(int64_t speed);
int gf_tm_delete_link();
int gf_tm_insert_link(char *staname);
int gf_tm_refresh_link(char *staname);
int gf_tm_refresh_unlink(char *staname);

int gf_tm_exist_task(char *sat, char *orb, char *state, char *statebackup);
int gf_tm_exist_chn(char *sat, char *orb, char *chn, char *state, char *statebackup);
int gf_tm_exist_file(char *sat, char *orb, char *chn, char *filename, char *state, char *statebackup, int *pPriority, int64_t *pSize);

int gf_tm_db_add_task(char *sat, char *orb, int chnnum);
int gf_tm_db_add_chn(char *sat, char *orb, char *chn);
int gf_tm_db_add_file(char *sat, char *orb, char *chn, char *filename, int priority, int64_t size);

/* 0-poll 1-complete 2-task error -1-error */
int gf_tm_check_poll_task(char *sat, char *orb);
/* 0-not done 1-done -1-error */
int gf_tm_check_task_done(char *sat, char *orb);
/* 0-not done 1-done -1-error */
int gf_tm_check_chn_done(char *sat, char *orb, char *chn);

int gf_tm_change_task_state(char *sat, char *orb, char *state, int backup);
int gf_tm_change_chn_state(char *sat, char *orb, char *chn, char *state, int backup);
int gf_tm_change_file_state(char *sat, char *orb, char *chn, char *filename, char *state, int backup);

int gf_tm_query_active_task(SQLHSTMT *phstmt);
int gf_tm_get_task(SQLHSTMT hstmt, gf_tm_task_t *ptask);
/* 0-not exist 1-success -1-error */
int gf_tm_get_handle_task(gf_tm_task_t *ptask);

int gf_tm_query_active_chn(SQLHSTMT *phstmt);
int gf_tm_get_chn(SQLHSTMT hstmt, gf_tm_chn_t *pchn);
/* 0-not exist 1-success -1-error */
int gf_tm_get_handle_chn(gf_tm_chn_t *pchn);

int gf_tm_query_trans_file(SQLHSTMT *phstmt);
int gf_tm_query_pause_file(SQLHSTMT *phstmt);
int gf_tm_get_file(SQLHSTMT hstmt, gf_tm_file_t *pfile);
/* 0-not exist 1-success -1-error */
int gf_tm_get_handle_file(gf_tm_file_t *pfile);

int gf_tm_add_task_retran(char *sat, char *orb);
int gf_tm_add_chn_retran(char *sat, char *orb, char *chn);
int gf_tm_add_file_retran(char *sat, char *orb, char *chn, char *filename);

int gf_tm_set_file_recv_size(char *sat, char *orb, char *chn, char *filename, int64_t recv_size);

int gf_tm_db_clean_task(char *sat, char *orb);
int gf_tm_db_clean_chn(char *sat, char *orb, char *chn);
int gf_tm_db_del_file(char *sat, char *orb, char *chn, char *filename);

int gf_tm_recover_state(void);

int gf_tm_refresh_task_donetime(char *sat, char *orb);

#endif /* __GFTMDB_H__ */
