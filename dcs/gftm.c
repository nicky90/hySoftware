/*
 * gftm.h - GF task manager function
 *
 * Writen by CX3201 2013-05-31
 */

#include "gftm.h"
#include "gftmdata.h"
#include "gferror.h"
#include "gftmdb.h"
#include "gftmconfig.h"
#include "gftmfile.h"
#include "gftmworkqueue.h"

/*---------------------- */
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <sys/socket.h>

#define UI_PORT		4747          // BJ listen PORT

typedef struct {
	char req_name[20];
	char req_args[5][50];
} ui_msg_t;

static int UI_MSG_SIZE = sizeof(ui_msg_t);
/*------------------------*/

gf_tm_runtime_t gf_tm_runtime;

int gf_tm_lock(void)
{
	if(pthread_mutex_lock(&(gf_tm_runtime.tm_lock))) GF_ERROR(GFLOCKERR);
	return 0;
}

int gf_tm_unlock(void)
{
	if(pthread_mutex_unlock(&(gf_tm_runtime.tm_lock))) GF_ERROR(GFUNLOCKERR);
	return 0;
}

int gf_tm_handle_lock(void)
{
	if(pthread_mutex_lock(&(gf_tm_runtime.handle_lock))) GF_ERROR(GFLOCKERR);
	return 0;
}

int gf_tm_handle_unlock(void)
{
	if(pthread_mutex_unlock(&(gf_tm_runtime.handle_lock))) GF_ERROR(GFUNLOCKERR);
	return 0;
}

int gf_tm_handle_wait(void)
{
	if(sem_wait(&(gf_tm_runtime.handle_sem))) GF_ERROR(GFLOCKERR);
	return 0;
}

int gf_tm_handle_post(void)
{
	if(sem_post(&(gf_tm_runtime.handle_sem))) GF_ERROR(GFUNLOCKERR);
	return 0;
}

int gf_tm_runtime_lock(void)
{
	if(pthread_mutex_lock(&(gf_tm_runtime.runtime_lock))) GF_ERROR(GFLOCKERR);
	return 0;
}

int gf_tm_runtime_unlock(void)
{
	if(pthread_mutex_unlock(&(gf_tm_runtime.runtime_lock))) GF_ERROR(GFUNLOCKERR);
	return 0;
}

void* gf_tm_statistics_thread(void *param)
{
	while(1){
		sleep(5);
		if(gf_tm_runtime_lock()) GF_ABORT(errno);
		gf_tm_refresh_speed(gf_tm_runtime.speed / 5);
		gf_tm_runtime.speed = 0;
		if(gf_tm_runtime_unlock()) GF_ABORT(errno);
	}
}

void* gf_tm_poll_thread(void *param)
{
	int                 ret;
	SQLRETURN           sret;
	SQLHSTMT            hstmt = NULL;
	gf_tm_sat_config_t *psat;
	gf_tm_sat_config_t *psatname;
	gf_tm_task_t        task;
	gf_tm_chn_t         chn;
	psatname=pgf_tm_sat_config;
	if(gf_tm_delete_link()){
	}
	while(psatname){
		if(gf_tm_insert_link(psatname->name)) GF_ERROR_LOG(errno);
		psatname = psatname->next;
	}
	while(1){
		if(gf_tm_runtime.may_exit) break;
		psat = pgf_tm_sat_config;
		while(psat){
			if(gf_tm_poll_sat(psat->name, gf_tm_add_task)){
				gf_tm_refresh_unlink(psat->name);
				GF_ERROR_LOG(errno);
				}
			else gf_tm_refresh_link(psat->name);
			
printf("Polling sat: %s\n", psat->name);
			psat = psat->next;
		}
		if(0 == gf_tm_query_active_task(&hstmt)){
			while(1){
				sret = SQLFetch(hstmt);
				if(SQL_NO_DATA == sret){
					break;
				}else if(!SQL_OK(sret)){
					GF_ERROR_LOG(GFDATAERR);
					gf_db_free_stmt(hstmt);
					break;
				}else{
					if(gf_tm_get_task(hstmt, &task)){
						GF_ERROR_LOG(errno);
						break;
					}
					if(gf_tm_poll_task(task.sat, task.orb, gf_tm_add_chn)) GF_ERROR_LOG(errno);
printf("Polling task: %s-%s\n", task.sat, task.orb);
				}
			}
			gf_db_free_stmt(hstmt);
		}
		if(0 == gf_tm_query_active_chn(&hstmt)){
			while(1){
				sret = SQLFetch(hstmt);
				if(SQL_NO_DATA == sret){
					break;
				}else if(!SQL_OK(sret)){
					GF_ERROR_LOG(GFDATAERR);
					gf_db_free_stmt(hstmt);
					break;
				}else{
					if(gf_tm_get_chn(hstmt, &chn)){
						GF_ERROR_LOG(errno);
						break;
					}
					ret = gf_tm_poll_chn(chn.sat, chn.orb, chn.chn, gf_tm_add_file);
					if(-1 == ret) GF_ERROR_LOG(errno)
					else if(0 < ret){
						if(gf_tm_lock()) GF_ABORT(errno);
						ret = gf_tm_check_chn_done(chn.sat, chn.orb, chn.chn);
						if(-1 == ret) GF_ERROR_LOG(errno);
						if(1 == ret){
							if(gf_tm_change_chn_state(chn.sat, chn.orb, chn.chn, GF_TM_STATE_HANDLE, 1))
								GF_ERROR_LOG(errno);
							if(gf_tm_handle_post()) GF_ERROR_LOG(errno);
						}else{
							if(gf_tm_change_chn_state(chn.sat, chn.orb, chn.chn, GF_TM_STATE_TRANSMIT, 1))
								GF_ERROR_LOG(errno);
						}
						if(gf_tm_unlock()) GF_ABORT(errno);
					}
printf("Polling chn: %s-%s-%s\n", chn.sat, chn.orb, chn.chn);
				}
			}
			gf_db_free_stmt(hstmt);
		}
		sleep(gf_tm_config.poll_period);
	}
	
	return NULL;
}

void* gf_tm_handle_thread(void *param)
{
	int          ret, status;
	gf_tm_file_t file;
	gf_tm_chn_t  chn;
	gf_tm_task_t task;
	gf_tm_sat_config_t *psat;
	
	while(1){
		if(gf_tm_runtime.may_exit) break;
		if(gf_tm_handle_wait()) GF_ABORT(errno);
		ret = 0;
		if(gf_tm_lock()) GF_ABORT(errno);
		if(1 == gf_tm_get_handle_task(&task)){
			psat = gf_tm_find_sat_config(task.sat);
			if(!psat) ret = -1;
			else if(psat->orbhandler[0]){
				gf_tm_runtime.handle_board.type = GF_TM_HANDLE_TASK;
				strcpy(gf_tm_runtime.handle_board.sat, task.sat);
				strcpy(gf_tm_runtime.handle_board.orb, task.orb);
				gf_tm_runtime.handle_board.chn[0] = '\0';
				gf_tm_runtime.handle_board.filename[0] = '\0';
				gf_tm_runtime.handle_board.pid = fork();
				if(gf_tm_runtime.handle_board.pid < 0){
					GF_ERROR_LOG(GFFORKERR);
					ret = -1;
				}else if(0 == gf_tm_runtime.handle_board.pid){
					execl(psat->orbhandler, GF_TM_TASK_HANDLER, GF_TM_TASK_PARAM(task), NULL);
					exit(-1);
				}else{
					ret = 1;
				}
			}else ret = 0;
		}else if(1 == gf_tm_get_handle_chn(&chn)){
			psat = gf_tm_find_sat_config(chn.sat);
			if(!psat) ret = -1;
			else if(psat->chnhandler[0]){
				gf_tm_runtime.handle_board.type = GF_TM_HANDLE_CHN;
				strcpy(gf_tm_runtime.handle_board.sat, chn.sat);
				strcpy(gf_tm_runtime.handle_board.orb, chn.orb);
				strcpy(gf_tm_runtime.handle_board.chn, chn.chn);
				gf_tm_runtime.handle_board.filename[0] = '\0';
				gf_tm_runtime.handle_board.pid = fork();
				if(gf_tm_runtime.handle_board.pid < 0){
					GF_ERROR_LOG(GFFORKERR);
					ret = -1;
				}else if(0 == gf_tm_runtime.handle_board.pid){
					execl(psat->chnhandler, GF_TM_FILE_HANDLER, GF_TM_CHN_PARAM(chn), NULL);
					exit(-1);
				}else{
					ret = 1;
				}
			}else ret = 0;
		}else if(1 == gf_tm_get_handle_file(&file)){
			psat = gf_tm_find_sat_config(file.sat);
			if(!psat) ret = -1;
			else if(psat->filehandler[0]){
				gf_tm_runtime.handle_board.type = GF_TM_HANDLE_FILE;
				strcpy(gf_tm_runtime.handle_board.sat, file.sat);
				strcpy(gf_tm_runtime.handle_board.orb, file.orb);
				strcpy(gf_tm_runtime.handle_board.chn, file.chn);
				strcpy(gf_tm_runtime.handle_board.filename, file.filename);
				gf_tm_runtime.handle_board.pid = fork();
				if(gf_tm_runtime.handle_board.pid < 0){
					GF_ERROR_LOG(GFFORKERR);
					ret = -1;
				}else if(0 == gf_tm_runtime.handle_board.pid){
					execl(psat->filehandler, GF_TM_FILE_HANDLER, GF_TM_FILE_PARAM(file), NULL);
					exit(-1);
				}else{
					ret = 1;
				}
			}else ret = 0;
		}else{
			ret = 0;
			gf_tm_runtime.handle_board.type = GF_TM_HANDLE_NONE;
		}
		if(gf_tm_handle_lock()) GF_ABORT(errno);
		if(gf_tm_unlock()) GF_ABORT(errno);
		
		if(1 == ret){
			waitpid(gf_tm_runtime.handle_board.pid, &status, 0);
			if(WIFEXITED(status)){
				if(0 == WEXITSTATUS(status)) ret = 0;
				else{
					GF_ERROR_LOG(GFEXECERR);
					ret = -1;
				}
			}else{
				GF_ERROR_LOG(GFABORT);
				ret = -1;
			}
		}
		
		if(GF_TM_HANDLE_TASK == gf_tm_runtime.handle_board.type){
			if(0 == ret){
				if(gf_tm_change_task_state(GF_TM_TASK_PARAM(task), GF_TM_STATE_DONE, 1)) GF_ABORT(errno);
				if(gf_tm_refresh_task_donetime(GF_TM_TASK_PARAM(task))) GF_ABORT(errno);
				gf_tm_log(GF_TM_TASK_PARAM(task), NULL, NULL, "Handle task", "Success");
			}else{
				if(gf_tm_change_task_state(GF_TM_TASK_PARAM(task), GF_TM_STATE_ERROR, 1)) GF_ABORT(errno);
				gf_tm_log(GF_TM_TASK_PARAM(task), NULL, NULL, "Handle task", "Error");
			}
		}else if(GF_TM_HANDLE_CHN == gf_tm_runtime.handle_board.type){
			if(0 == ret){
				if(gf_tm_change_chn_state(GF_TM_CHN_PARAM(chn), GF_TM_STATE_DONE, 1)) GF_ABORT(errno);
				gf_tm_log(GF_TM_CHN_PARAM(chn), NULL, "Handle channel", "Success");
				ret = gf_tm_check_task_done(chn.sat, chn.orb);
				if(-1 == ret) GF_ABORT(errno);
				if(1 == ret){
					if(gf_tm_change_task_state(chn.sat, chn.orb, GF_TM_STATE_HANDLE, 1)) GF_ABORT(errno);
					if(gf_tm_handle_post()) GF_ABORT(errno);
				}
			}else{
				if(gf_tm_change_chn_state(GF_TM_CHN_PARAM(chn), GF_TM_STATE_ERROR, 1)) GF_ABORT(errno);
				gf_tm_log(GF_TM_CHN_PARAM(chn), NULL, "Handle channel", "Error");
			}
		}else if(GF_TM_HANDLE_FILE == gf_tm_runtime.handle_board.type){
			if(0 == ret){
				if(gf_tm_change_file_state(GF_TM_FILE_PARAM(file), GF_TM_STATE_DONE, 1)) GF_ABORT(errno);
				gf_tm_log(GF_TM_FILE_PARAM(file), "Handle file", "Success");
				ret = gf_tm_check_chn_done(file.sat, file.orb, file.chn);
				if(-1 == ret) GF_ABORT(errno);
				if(1 == ret){
					if(gf_tm_change_chn_state(file.sat, file.orb, file.chn, GF_TM_STATE_HANDLE, 1)) GF_ABORT(errno);
					if(gf_tm_handle_post()) GF_ABORT(errno);
				}
			}else{
				if(gf_tm_change_file_state(GF_TM_FILE_PARAM(file), GF_TM_STATE_ERROR, 1)) GF_ABORT(errno);
				gf_tm_log(GF_TM_FILE_PARAM(file), "Handle file", "Error");
			}
		}
		
		gf_tm_runtime.handle_board.type = GF_TM_HANDLE_NONE;
		gf_tm_runtime.handle_board.sat[0] = '\0';
		gf_tm_runtime.handle_board.orb[0] = '\0';
		gf_tm_runtime.handle_board.chn[0] = '\0';
		gf_tm_runtime.handle_board.filename[0] = '\0';
		gf_tm_runtime.handle_board.pid = -1;
		if(gf_tm_handle_unlock()) GF_ABORT(errno);
	}

	return NULL;
}

int gf_tm_init_runtime(void)
{
	gf_tm_runtime.speed = 0;
	gf_tm_runtime.may_exit = 0;
	memset(&(gf_tm_runtime.handle_board), 0, sizeof(gf_tm_handle_board_t));
	if(pthread_mutex_init(&(gf_tm_runtime.tm_lock), NULL)) GF_ERROR(GFALLOCERR);
	if(pthread_mutex_init(&(gf_tm_runtime.runtime_lock), NULL)) GF_ERROR(GFALLOCERR);
	if(pthread_mutex_init(&(gf_tm_runtime.handle_lock), NULL)) GF_ERROR(GFALLOCERR);
	if(sem_init(&(gf_tm_runtime.handle_sem), 0, 0)) GF_ERROR(GFALLOCERR);
	if(pthread_create(&(gf_tm_runtime.statistics_thread), NULL, gf_tm_statistics_thread, NULL)) GF_ERROR(GFMAKEERR);
	if(pthread_create(&(gf_tm_runtime.poll_thread), NULL, gf_tm_poll_thread, NULL)) GF_ERROR(GFMAKEERR);
	if(pthread_create(&(gf_tm_runtime.handle_thread), NULL, gf_tm_handle_thread, NULL)) GF_ERROR(GFMAKEERR);

	return 0;
}

int gf_tm_destroy_runtime(void)
{
	gf_tm_runtime.may_exit = 1;
	gf_tm_lock();
	if(gf_tm_runtime.handle_board.pid >0) kill(gf_tm_runtime.handle_board.pid, SIGKILL);
	gf_tm_handle_lock();
	sem_destroy(&(gf_tm_runtime.handle_sem));
	pthread_mutex_destroy(&(gf_tm_runtime.tm_lock));
	pthread_mutex_destroy(&(gf_tm_runtime.handle_lock));
	pthread_mutex_destroy(&(gf_tm_runtime.runtime_lock));

	return 0;
}

int gf_tm_add_speed(int64_t val)
{
	if(gf_tm_runtime_lock()) GF_ERROR(errno);
	gf_tm_runtime.speed += val;
	if(gf_tm_runtime_unlock()) GF_ERROR(errno);
	return 0;
}

int gf_tm_zero_speed(void)
{
	if(gf_tm_runtime_lock()) GF_ERROR(errno);
	gf_tm_runtime.speed = 0;
	if(gf_tm_runtime_unlock()) GF_ERROR(errno);
	return 0;
}

int gf_tm_get_speed(int64_t *pval)
{
	if(gf_tm_runtime_lock()) GF_ERROR(errno);
	*pval = gf_tm_runtime.speed;
	if(gf_tm_runtime_unlock()) GF_ERROR(errno);
	return 0;
}

int gf_tm_destroy(char *state)
{
	/* Wait threads exit */
	gf_tm_destroy_work_threads();
	/* Release */
	gf_tm_destroy_workspace_queue();
	gf_tm_destroy_work_slot();
	gf_tm_destroy_config();
	gf_tm_refresh_state(state);
	gf_tm_refresh_speed(0);
	gf_tm_destroy_runtime();
	gf_db_disconn();

	return 0;
}

int gf_tm_init(char *dsn, char *usr, char *psw)
{
	/* Database init */
	if(gf_db_init(dsn, usr, psw)) GF_ERROR_PR(errno);
	if(gf_db_conn()) GF_ERROR_PR(errno);
//gf_tm_init_tables();
	if(gf_tm_recover_state()) GF_ERROR(errno);
	/* Config init */
	if(gf_tm_read_config(GF_TM_CONFIG_FILE)){
		if(gf_db_disconn()) GF_ERROR(errno);
		GF_ERROR(errno);
	}
	if(gf_tm_read_sat_config(GF_TM_SAT_CONFIG_FILE)){
		if(gf_db_disconn()) GF_ERROR(errno);
		GF_ERROR(errno);
	}
	/* Runtime init */
	if(gf_tm_init_runtime()){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	/* State init */
	if(gf_tm_refresh_starttime()){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	if(gf_tm_refresh_state("RUNNING")){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	if(gf_tm_refresh_speed(0)){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	if(gf_tm_alloc_work_slot()){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	if(gf_tm_init_workspace_queue()){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	if(gf_tm_make_work_threads()){
		GF_ERROR(errno);
		gf_tm_destroy("ERROR");
	}
	gf_tm_data_broadcast();
	
	return 0;
}

/*************Main*************/
int main(int argc, char *argv[])
{
	struct sockaddr_in ui_addr;
	int ui_listfd;
	int on = 1;
	ui_msg_t *buf;

	ui_listfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(ui_listfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&ui_addr, 0, sizeof(ui_addr));
	ui_addr.sin_family = AF_INET;
	ui_addr.sin_port = htons(UI_PORT);
	ui_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(ui_listfd, (struct sockaddr *)&ui_addr, sizeof(ui_addr));
	listen(ui_listfd, 10);

	if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) GF_ERROR_PR(errno);

	for ( ; ; )
	{
	    int socklen, connfd;
	    struct sockaddr_in ui_connaddr;
		socklen = sizeof(ui_connaddr);
		connfd = accept(ui_listfd, (struct sockaddr *)&ui_connaddr, (socklen_t *)&socklen);
		if (connfd == -1) {
			perror("accept");
			continue;
		}
		buf = (ui_msg_t *)malloc(UI_MSG_SIZE);
		memset(buf, '\0', UI_MSG_SIZE);
		if (read(connfd, buf, UI_MSG_SIZE)>0) {
				// remember free buf in each switch
			if (!strncmp(buf->req_name, "pause", 5)) {
				// validate the args
				if ((strncmp(buf->req_args[0], "GF01", 4)&&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, "UI_PAUSE: wrong arguments");
					free(buf);
					continue;
				}
				//printf("pause:\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1]);
				gf_tm_pause_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
				//sleep(1);
			}
			else if (!strncmp(buf->req_name, "resume", 6)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, "UI_RESUME: wrong arguments");
					free(buf);
					continue;
				}
				//printf("resume:\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1]);
				gf_tm_resume_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "cancel", 6)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, "UI_CANCEL: wrong arguments");
					free(buf);
					continue;
				}
				//printf("cancel:\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1]);
				gf_tm_cancel_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "correct", 7)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, "UI_CORRECT: wrong arguments");
					free(buf);
					continue;
				}
				printf("correct:\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "orbretran", 10)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, "UI_ORBRETRAN: wrong arguments");
					free(buf);
					continue;
				}
				//printf("orbretran:\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1]);
				gf_tm_retran_task(buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "chnretran", 9)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0)
				) {
					fprintf(stderr, "UI_CHNRETRAN: wrong arguments");
					free(buf);
					continue;
				}
				//printf("chnretran:\n\t%s\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1], buf->req_args[2]);
				gf_tm_retran_chn(buf->req_args[0], buf->req_args[1], buf->req_args[2]);

				free(buf);
			}
			else if (!strncmp(buf->req_name, "fileretran", 10)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0) || \
					(strlen(buf->req_args[3]) == 0) 
				) {
					fprintf(stderr, "UI_FILERETRAN: wrong arguments");
					free(buf);
					continue;
				}
				//printf("fileretran:\n\t%s\n\t%s\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1], buf->req_args[2], buf->req_args[3]);
				gf_tm_retran_file(buf->req_args[0], buf->req_args[1], buf->req_args[2], buf->req_args[3]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "addtask", 6)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0)
				) {
					fprintf(stderr, "UI_ADDTASK: wrong arguments");
					free(buf);
					continue;
				}
				printf("addtask:\n\t%s\n\t%s\n", buf->req_args[0], buf->req_args[1]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "addchn", 6)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0)
				) {
					fprintf(stderr, "UI_ADDCHN: wrong arguments");
					free(buf);
					continue;
				}
				printf("addchn:\n\t%s\n\t%s\n\t%s\n", buf->req_args[0], \
					buf->req_args[1], buf->req_args[2]);
				free(buf);
			}
			else if (!strncmp(buf->req_name, "addfile", 7)) {
				if ( 
					(strncmp(buf->req_args[0], "GF01", 4) &&
					strncmp(buf->req_args[0], "GF02", 4) &&
					strncmp(buf->req_args[0], "GF03", 4)) || \
					(strlen(buf->req_args[1]) == 0) || \
					(strlen(buf->req_args[2]) == 0) || \
					(strlen(buf->req_args[3]) == 0)
				) {
					fprintf(stderr,	"UI_ADDFILE: wrong arguments");
					free(buf);
					continue;
				}
				printf("addfile:\n\t%s\n\t%s\n\t%s\n\t%s\n", buf->req_args[0], \
					buf->req_args[1], \
					buf->req_args[2], buf->req_args[3]);
				free(buf);
			}
			else {
				printf("unknown request from ui\n");
				free(buf);
			}
		}
		else {
			printf("receive msg failed\n");
			free(buf);
		}
		close(connfd);
	}

	exit(0);
}

/***************DEBUG********************/
void gf_tm_print_file(gf_tm_file_t *pfile)
{
	if(!pfile) return;
	printf("%s:%s:%s:%s:%s:%s:%d:%lld:%lld:%d\n",
		pfile->state, pfile->sat, pfile->orb,
		pfile->chn, pfile->filename, pfile->statebackup,
		pfile->priority,
		(long long int)pfile->size,
		(long long int)pfile->recv_size,
		pfile->retran);
}
