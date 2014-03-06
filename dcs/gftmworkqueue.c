/*
 * gftm.h - GF task manager work queue
 *
 * Writen by CX3201 2013-06-20
 */

#include "gftmworkqueue.h"
#include "gftmfile.h"
#include "gferror.h"
#include "gftmconfig.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include "gftmdata.h"
#include <signal.h>

static gf_tm_workspace_t gf_tm_trans_queue, gf_tm_pause_queue;
static gf_tm_work_t *gf_tm_work_slot;

static void gf_tm_workspace_insert(gf_tm_workspace_t *head, gf_tm_workspace_t *dst)
{
	gf_tm_workspace_t *prev, *cur;

	prev = head; cur = prev->next;
	while(cur && cur->file.priority <= dst->file.priority){
		prev = cur; cur = prev->next;
	}
	dst->next = cur;
	prev->next = dst;
}

static gf_tm_workspace_t* gf_tm_find_workspace(char *sat, char *orb, char *chn, char *filename)
{
	gf_tm_workspace_t *p = NULL;
	
	if(!sat || !orb || !chn || !filename) return NULL;
	p = gf_tm_trans_queue.next;
	while(p){
		if(strcmp(sat, p->file.sat) == 0 &&\
			strcmp(orb, p->file.orb) == 0 &&\
			strcmp(chn, p->file.chn) == 0 &&\
			strcmp(filename, p->file.filename) == 0)
			return p;
		p = p->next;
	}
	p = gf_tm_pause_queue.next;
	while(p){
		if(strcmp(sat, p->file.sat) == 0 &&\
			strcmp(orb, p->file.orb) == 0 &&\
			strcmp(chn, p->file.chn) == 0 &&\
			strcmp(filename, p->file.filename) == 0)
			return p;
		p = p->next;
	}
	return NULL;
}

static int gf_tm_open_workspace(gf_tm_file_t *pfile, int recreate, int queue)
{
	int     tmp;
	int64_t ret, bsize, i;
	struct  stat st;
	gf_tm_sat_config_t *satconf = NULL;
	gf_tm_workspace_t *p = NULL;
	
	if(!pfile) GF_ERROR(GFNULLPARAM);
	if(queue != GF_TM_TRANS_QUEUE && queue != GF_TM_PAUSE_QUEUE) GF_ERROR(GFINVAPARAM);
	satconf = gf_tm_find_sat_config(pfile->sat);
	if(!satconf) GF_ERROR(errno);
	p = gf_tm_find_workspace(pfile->sat, pfile->orb, pfile->chn, pfile->filename);
	if(p) GF_ERROR(GFEXIST);
	
	p = (gf_tm_workspace_t*)malloc(sizeof(gf_tm_workspace_t));
	if(!p) GF_ERROR(GFALLOCERR);
	
	memcpy(&(p->file), pfile, sizeof(gf_tm_file_t));
	if(gf_tm_make_file_path(p->datapath, p->file.sat,p->file.orb,p->file.chn,p->file.filename, NULL)){
		free(p);
		GF_ERROR(GFMAKEERR);
	}
	if(gf_tm_make_file_path(p->statuspath, p->file.sat,p->file.orb,p->file.chn,p->file.filename, GF_TM_STATUS_FILE_SUFFIX)){
		free(p);
		GF_ERROR(GFMAKEERR);
	}
	p->alloc_index = 0;
	p->done_count = 0;
	p->doing_count = 0;
	p->error_count = 0;
	p->file.recv_size = 0;
	p->check = satconf->check;
	p->compress = satconf->compress;

	if(!recreate){
		if(lstat(p->datapath, &st)) recreate = 1;
		else if(!S_ISREG(st.st_mode)){
			free(p);
			GF_ERROR(GFOPENERR);
		}
		if(lstat(p->statuspath, &st)) recreate = 1;
		else if(!S_ISREG(st.st_mode)){
			free(p);
			GF_ERROR(GFOPENERR);
		}
	}
gf_tm_open_flag:
	p->statusfd = gf_tm_open_file(p->statuspath, recreate);
	if(-1 == p->statusfd){
		free(p);
		GF_ERROR(errno);
	}
	p->datafd = gf_tm_open_file(p->datapath, recreate);
	if(-1 == p->datafd){
		tmp = errno;
		close(p->statusfd);
		free(p);
		GF_ERROR(tmp);
	}

	if(recreate){
		ret = pwrite(p->statusfd, &(p->file.size), sizeof(int64_t), 0);
		if(sizeof(int64_t) != ret){
			close(p->datafd);
			close(p->statusfd);
			free(p);
			GF_ERROR(GFSTATEERR);
		}
		ret = pwrite(p->statusfd, &(gf_tm_config.block_size), sizeof(int64_t), sizeof(int64_t));
		if(sizeof(int64_t) != ret){
			close(p->datafd);
			close(p->statusfd);
			free(p);
			GF_ERROR(GFSTATEERR);
		}
		bsize = gf_tm_config.block_size;
		p->block_size = bsize;
		p->block_count = (p->file.size - 1) / bsize + 1;
		p->block_state = (uint8_t*)calloc(1, p->block_count);
		if(!p->block_state){
			close(p->datafd);
			close(p->statusfd);
			free(p);
			GF_ERROR(GFALLOCERR);
		}
		ret = pwrite(p->statusfd, p->block_state, p->block_count, sizeof(int64_t) + sizeof(int64_t));
		if(p->block_count != ret){
			close(p->datafd);
			close(p->statusfd);
			free(p->block_state);
			free(p);
			GF_ERROR(GFRDNERR);
		}
	}else{
		ret = pread(p->statusfd, &bsize, sizeof(int64_t), 0);
		if(sizeof(int64_t) != ret || bsize != p->file.size){
			close(p->datafd);
			close(p->statusfd);
			recreate = 1;
			goto gf_tm_open_flag;
		}
		ret = pread(p->statusfd, &bsize, sizeof(int64_t), sizeof(int64_t));
		if(sizeof(int64_t) != ret){
			close(p->datafd);
			close(p->statusfd);
			recreate = 1;
			goto gf_tm_open_flag;
		}
		p->block_size = bsize;
		p->block_count = (p->file.size - 1) / bsize + 1;
		p->block_state = (uint8_t*)malloc(p->block_count);
		if(!p->block_state){
			close(p->datafd);
			close(p->statusfd);
			free(p);
			GF_ERROR(GFALLOCERR);
		}
		ret = pread(p->statusfd, p->block_state, p->block_count, sizeof(int64_t) + sizeof(int64_t));
		if(p->block_count != ret){
			close(p->datafd);
			close(p->statusfd);
			free(p->block_state);
			recreate = 1;
			goto gf_tm_open_flag;
		}
		for(i = 0; i < p->block_count; i++){
			if(GF_TM_BLOCK_DONE == p->block_state[i]){
				p->done_count++;
				if(i != p->block_count - 1) p->file.recv_size += p->block_size;
				else p->file.recv_size += (p->file.size - 1) % p->block_size + 1;
			}
		}
	}
	if(gf_tm_set_file_recv_size(GF_TM_FILE_PARAM(p->file), p->file.recv_size)){
		close(p->datafd);
		close(p->statusfd);
		free(p->block_state);
		free(p);
		GF_ERROR(errno);
	}
	if(GF_TM_TRANS_QUEUE == queue) gf_tm_workspace_insert(&gf_tm_trans_queue, p);
	else gf_tm_workspace_insert(&gf_tm_pause_queue, p);

	return 0;
}

static int gf_tm_close_workspace(char *sat, char *orb, char *chn, char *filename)
{
	int i, count = 0;
	gf_tm_workspace_t *p = NULL, *prev = NULL;
	
	prev = &gf_tm_trans_queue;
	p = gf_tm_trans_queue.next;
	while(p){
		if( (!sat || strcmp(sat, p->file.sat) == 0) &&\
			(!orb || strcmp(orb, p->file.orb) == 0) &&\
			(!chn || strcmp(chn, p->file.chn) == 0) &&\
			(!filename || strcmp(filename, p->file.filename) == 0)){
				/* Abort doing threads */
				for(i = 0; i < gf_tm_config.thread_num; i++){
					if(gf_tm_work_slot[i].workspace == p){
						gf_tm_work_slot[i].state = GF_TM_WORK_ABORT;
						gf_tm_work_slot[i].workspace = NULL;
					}
				}
				close(p->datafd);
				close(p->statusfd);
				if(p->block_state) free(p->block_state);
				prev->next = p->next;
				free(p);
				p = prev->next;
				count++;
		}else{
			prev = p;
			p = p->next;
		}
	}
	prev = &gf_tm_pause_queue;
	p = gf_tm_pause_queue.next;
	while(p){
		if( (!sat || strcmp(sat, p->file.sat) == 0) &&\
			(!orb || strcmp(orb, p->file.orb) == 0) &&\
			(!chn || strcmp(chn, p->file.chn) == 0) &&\
			(!filename || strcmp(filename, p->file.filename) == 0)){
				/* Abort doing threads */
				for(i = 0; i < gf_tm_config.thread_num; i++){
					if(gf_tm_work_slot[i].workspace == p){
						gf_tm_work_slot[i].state = GF_TM_WORK_ABORT;
						gf_tm_work_slot[i].workspace = NULL;
					}
				}
				close(p->datafd);
				close(p->statusfd);
				if(p->block_state) free(p->block_state);
				prev->next = p->next;
				free(p);
				p = prev->next;
				count++;
		}else{
			prev = p;
			p = p->next;
		}
	}
	return count;
}

static int gf_tm_pause_workspace(char *sat, char *orb, char *chn, char *filename)
{
	int count = 0;
	gf_tm_workspace_t *p = NULL, *prev = NULL;
	
	prev = &gf_tm_trans_queue;
	p = gf_tm_trans_queue.next;
	while(p){
		if( (!sat || strcmp(sat, p->file.sat) == 0) &&\
			(!orb || strcmp(orb, p->file.orb) == 0) &&\
			(!chn || strcmp(chn, p->file.chn) == 0) &&\
			(!filename || strcmp(filename, p->file.filename) == 0)){
				prev->next = p->next;
				gf_tm_workspace_insert(&gf_tm_pause_queue, p);
				p = prev->next;
				count++;
		}else{
			prev = p;
			p = p->next;
		}
	}
	return count;
}

static int gf_tm_resume_workspace(char *sat, char *orb, char *chn, char *filename)
{
	int count = 0;
	gf_tm_workspace_t *p = NULL, *prev = NULL;
	
	prev = &gf_tm_pause_queue;
	p = gf_tm_pause_queue.next;
	while(p){
		if( (!sat || strcmp(sat, p->file.sat) == 0) &&\
			(!orb || strcmp(orb, p->file.orb) == 0) &&\
			(!chn || strcmp(chn, p->file.chn) == 0) &&\
			(!filename || strcmp(filename, p->file.filename) == 0)){
				prev->next = p->next;
				gf_tm_workspace_insert(&gf_tm_trans_queue, p);
				p = prev->next;
				count++;
		}else{
			prev = p;
			p = p->next;
		}
	}
	return count;
}

int gf_tm_init_workspace_queue(void)
{
	SQLRETURN    sret;
	SQLHSTMT     hstmt = NULL;
	int          ret;
	gf_tm_file_t file;

	memset(&gf_tm_trans_queue, 0, sizeof(gf_tm_workspace_t));
	memset(&gf_tm_pause_queue, 0, sizeof(gf_tm_workspace_t));

	if(gf_tm_query_trans_file(&hstmt)) GF_ERROR(errno);
	while(1){
		sret = SQLFetch(hstmt);
		if(SQL_NO_DATA == sret){
			ret = 0;
			break;
		}else if(!SQL_OK(sret)){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}else{
			ret = gf_tm_get_file(hstmt, &file);
			if(ret){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
			if(gf_tm_open_workspace(&file, 0, GF_TM_TRANS_QUEUE)){
				GF_ERROR_LOG(errno);
				if(gf_tm_change_file_state(GF_TM_FILE_PARAM(file), GF_TM_STATE_ERROR, 1))
					GF_ERROR(errno);
				continue;
			}
		}
	}
	gf_db_free_stmt(hstmt);
	
	if(gf_tm_query_pause_file(&hstmt)) GF_ERROR(errno);
	while(1){
		sret = SQLFetch(hstmt);
		if(SQL_NO_DATA == sret){
			ret = 0;
			break;
		}else if(!SQL_OK(sret)){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}else{
			ret = gf_tm_get_file(hstmt, &file);
			if(ret){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
			if(gf_tm_open_workspace(&file, 0, GF_TM_PAUSE_QUEUE)){
				GF_ERROR_LOG(errno);
				if(gf_tm_change_file_state(GF_TM_FILE_PARAM(file), GF_TM_STATE_ERROR, 1))
					GF_ERROR(errno);
				continue;
			}
		}
	}
	gf_db_free_stmt(hstmt);
	
	return ret;
}

void gf_tm_destroy_workspace_queue(void)
{
	gf_tm_close_workspace(NULL, NULL, NULL, NULL);
}

int gf_tm_alloc_work_slot(void)
{
	gf_tm_work_slot = (gf_tm_work_t*)calloc(gf_tm_config.thread_num, sizeof(gf_tm_work_t));
	if(!gf_tm_work_slot) GF_ERROR(GFALLOCERR);

	return 0;
}

void gf_tm_destroy_work_slot(void)
{
	if(gf_tm_work_slot) free(gf_tm_work_slot);
}

/* 0-no work  1-success 2-collect file -1-error */
static int gf_tm_get_work(int id)
{
	int64_t alloc_size;
	gf_tm_workspace_t *pw = gf_tm_trans_queue.next, *tmp;

	while(pw){
		if(pw->error_count > gf_tm_config.retry){ /* Error occur */
			tmp = pw->next;
			if(gf_tm_change_file_state(GF_TM_FILE_PARAM(pw->file), GF_TM_STATE_ERROR, 1)) GF_ERROR(errno);
			gf_tm_close_workspace(GF_TM_FILE_PARAM(pw->file));
			gf_tm_log(GF_TM_FILE_PARAM(pw->file), "File transmit", "Error count limit exceed");
			pw = tmp;
			continue;
		}

		while(pw->alloc_index < pw->block_count){
			if(0 == pw->block_state[pw->alloc_index]){
				gf_tm_work_slot[id].workspace = pw;
				memcpy(&(gf_tm_work_slot[id].file), &(pw->file), sizeof(gf_tm_file_t));
				gf_tm_work_slot[id].block_index = pw->alloc_index;
				gf_tm_work_slot[id].offset = pw->alloc_index * pw->block_size;
				alloc_size = gf_tm_work_slot[id].size;
				gf_tm_work_slot[id].size = pw->file.size - gf_tm_work_slot[id].offset;
				if(gf_tm_work_slot[id].size > pw->block_size) gf_tm_work_slot[id].size = pw->block_size;
				if(gf_tm_work_slot[id].size > alloc_size){
					if(gf_tm_work_slot[id].buffer) free(gf_tm_work_slot[id].buffer);
					gf_tm_work_slot[id].buffer = (uint8_t*)malloc(gf_tm_work_slot[id].size);
					if(!gf_tm_work_slot[id].buffer) GF_ERROR(GFALLOCERR);
				}
				gf_tm_work_slot[id].dup_data_fd = dup(pw->datafd);
				if(-1 == gf_tm_work_slot[id].dup_data_fd) GF_ERROR(GFOPENERR);
				gf_tm_work_slot[id].check = pw->check;
				gf_tm_work_slot[id].compress = pw->compress;
printf("T%d Alloc index %ld\n", id, pw->alloc_index);
				pw->alloc_index++;
				pw->doing_count++;
				return 1;
			}
			pw->alloc_index++;
		}
		/* Check file state */
		if(pw->done_count == pw->block_count){ /* Transmit done */
			if(gf_tm_change_file_state(GF_TM_FILE_PARAM(pw->file), GF_TM_STATE_HANDLE, 1)) GF_ERROR(errno);
			gf_tm_log(GF_TM_FILE_PARAM(pw->file), "File transmit", "Success");
			gf_tm_close_workspace(GF_TM_FILE_PARAM(pw->file));
			if(gf_tm_handle_post()) GF_ERROR(errno);
			return 2;
		}else if(0 == pw->doing_count) pw->alloc_index = 0;
		else pw = pw->next;
	}
	return 0;
}

static int gf_tm_commit_work(int id, int state)
{
	int64_t ret;
	gf_tm_workspace_t *pw = gf_tm_work_slot[id].workspace;

	pw->doing_count--;
	if(GF_TM_BLOCK_DONE == state){
		pw->block_state[gf_tm_work_slot[id].block_index] = state;
		ret = pwrite(pw->statusfd, pw->block_state + gf_tm_work_slot[id].block_index,\
			1, sizeof(int64_t) + sizeof(int64_t) + gf_tm_work_slot[id].block_index);
		if(1 != ret) GF_ERROR(GFWRNERR);
		pw->file.recv_size += gf_tm_work_slot[id].size;
		if(gf_tm_set_file_recv_size(GF_TM_FILE_PARAM(pw->file), pw->file.recv_size)) GF_ERROR(errno);
		/* Modify speed */
		gf_tm_add_speed(gf_tm_work_slot[id].size);
		pw->done_count++;
	}else pw->error_count++;

	return 0;
}

void* gf_tm_work_thread(void *param)
{
	int64_t sret;
	int ret, state;
	int id = (int)(int64_t)param;

	while(1){
		if(gf_tm_work_slot[id].dup_data_fd > 0){
			close(gf_tm_work_slot[id].dup_data_fd);
			gf_tm_work_slot[id].dup_data_fd = 0;
		}
		if(gf_tm_data_wait(id)) GF_ABORT(errno); /* Wait new work */
		if(gf_tm_lock()) GF_ABORT(errno);
		if(GF_TM_WORK_ABORT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			gf_tm_work_slot[id].state = 0;
			if(gf_tm_data_post(id)) GF_ABORT(errno);
			continue;
		}else if(GF_TM_WORK_EXIT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			break;
		}
		/* Get work */
		//printf("Thread %d get work\n", id);
		ret = gf_tm_get_work(id);
printf("T%d Get index %ld\n", id, gf_tm_work_slot[id].block_index);
		if(GF_TM_WORK_ABORT == gf_tm_work_slot[id].state){
gf_tm_print_workslots();
			if(gf_tm_unlock()) GF_ABORT(errno);
			gf_tm_work_slot[id].state = 0;
			if(gf_tm_data_post(id)) GF_ABORT(errno);
			continue;
		}else if(GF_TM_WORK_EXIT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			break;
		}
		if(gf_tm_unlock()) GF_ABORT(errno);

		if(-1 == ret) GF_ABORT(errno); /* Get block error is unrecoverable */
		if(0 == ret) continue;
		if(2 == ret){
			if(gf_tm_data_post(id)) GF_ABORT(errno);
			continue;
		}

		/* Do work */
		/** Get data **/
		//printf("Thread %d do work:%ld<%ld,%ld -> %d>\n", id, gf_tm_work_slot[id].block_index,
		//	gf_tm_work_slot[id].offset, gf_tm_work_slot[id].size, gf_tm_work_slot[id].dup_data_fd);
printf("T%d Do index %ld\n", id, gf_tm_work_slot[id].block_index);

		ret = gf_tm_get_block(gf_tm_work_slot[id].buffer, GF_TM_FILE_PARAM(gf_tm_work_slot[id].file), gf_tm_work_slot[id].offset,\
			gf_tm_work_slot[id].size, gf_tm_work_slot[id].check, gf_tm_work_slot[id].compress);
		state = GF_TM_BLOCK_ERROR;
		if(0 == ret){
			/** Write data **/
			sret = pwrite(gf_tm_work_slot[id].dup_data_fd, gf_tm_work_slot[id].buffer,\
					gf_tm_work_slot[id].size, gf_tm_work_slot[id].offset);
			if(sret == gf_tm_work_slot[id].size){
				fsync(gf_tm_work_slot[id].dup_data_fd);
				state = GF_TM_BLOCK_DONE;
			}
			close(gf_tm_work_slot[id].dup_data_fd);
			gf_tm_work_slot[id].dup_data_fd = 0;
		}

		if(gf_tm_lock()) GF_ABORT(errno);
		if(GF_TM_WORK_ABORT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			gf_tm_work_slot[id].state = 0;
			if(gf_tm_data_post(id)) GF_ABORT(errno);
			continue;
		}else if(GF_TM_WORK_EXIT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			break;
		}
		/* Commit work */
		//printf("Thread %d commit work\n", id);
printf("T%d Commit index %ld\n", id, gf_tm_work_slot[id].block_index);
		ret = gf_tm_commit_work(id, state);
		if(-1 == ret) GF_ABORT(errno); /* Commit block error is unrecoverable */

		if(GF_TM_WORK_ABORT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			gf_tm_work_slot[id].state = 0;
			if(gf_tm_data_post(id)) GF_ABORT(errno);
			continue;
		}else if(GF_TM_WORK_EXIT == gf_tm_work_slot[id].state){
			if(gf_tm_unlock()) GF_ABORT(errno);
			break;
		}
		if(gf_tm_unlock()) GF_ABORT(errno);
		if(gf_tm_data_post(id)) GF_ABORT(errno);
	}
	if(gf_tm_work_slot[id].dup_data_fd > 0){
		close(gf_tm_work_slot[id].dup_data_fd);
		gf_tm_work_slot[id].dup_data_fd = 0;
	}
	printf("T%d exit\n", (int)(int64_t)param);
	return NULL;
}

int gf_tm_data_wait(int id)
{
	if(sem_wait(&(gf_tm_work_slot[id].data_sem))) GF_ERROR(GFLOCKERR);
	return 0;
}

int gf_tm_data_post(int id)
{
	if(sem_post(&(gf_tm_work_slot[id].data_sem))) GF_ERROR(GFUNLOCKERR);
	return 0;
}

int gf_tm_data_broadcast(void)
{
	int i;
	for(i = 0; i < gf_tm_config.thread_num; i++)
		if(sem_post(&(gf_tm_work_slot[i].data_sem))) GF_ERROR(GFUNLOCKERR);
	return 0;
}

int gf_tm_make_work_threads(void)
{
	int i;

	for(i = 0; i < gf_tm_config.thread_num; i++){
		if(sem_init(&(gf_tm_work_slot[i].data_sem), 0, 0)) GF_ERROR(GFALLOCERR);
		if(pthread_create(&(gf_tm_work_slot[i].work_thread), NULL, gf_tm_work_thread, (void*)(int64_t)i)) GF_ERROR(GFMAKEERR);
	}
	return 0;
}

void gf_tm_destroy_work_threads(void)
{
	int i;

	gf_tm_lock();
	for(i = 0; i < gf_tm_config.thread_num; i++){
		gf_tm_work_slot[i].state = GF_TM_WORK_EXIT;
		gf_tm_data_post(i);
	}
	gf_tm_unlock();
	for(i = 0; i < gf_tm_config.thread_num; i++){
		pthread_join(gf_tm_work_slot[i].work_thread, NULL);
		sem_destroy(&(gf_tm_work_slot[i].data_sem));
	}
}

int gf_tm_add_task(char *sat, char *orb)
{
	int ret;
	gf_tm_sat_config_t *psat;

	psat = gf_tm_find_sat_config(sat);
	if(!psat){
		gf_tm_log(sat, orb, NULL, NULL, "Add task", "Satlite not exists");
		GF_ERROR(GFNEXIST);
	}
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, NULL, NULL, "Add task", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, NULL, NULL);
	if(0 == ret){
		ret = gf_tm_db_add_task(sat, orb, psat->chn_num);
		if(-1 == ret) gf_tm_log(sat, orb, NULL, NULL, "Add task", "Add task to database failed");
	}else if(-1 == ret) gf_tm_log(sat, orb, NULL, NULL, "Add task", "Check task exists failed");
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, NULL, NULL, "Add task", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) GF_ERROR(errno);
	if(0 == ret) gf_tm_log(sat, orb, NULL, NULL, "Add task", "Success");

	return 0;
}

int gf_tm_add_chn(char *sat, char *orb, char *chn)
{
	int  ret;
	char state[GF_TM_STATE_STR_MAX+1];
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, chn, NULL, "Add channel", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(1 == ret){
		if(GF_TM_ADD_ALLOW(state)){
			ret = gf_tm_exist_chn(sat, orb, chn, NULL, NULL);
			if(0 == ret){
				ret = gf_tm_db_add_chn(sat, orb, chn);
				if(0 == ret){
					ret = gf_tm_check_poll_task(sat, orb);
					if(-1 == ret){
						gf_tm_close_workspace(sat, orb, NULL, NULL);
						gf_tm_log(sat, orb, chn, NULL, "Add channel", "Check poll failed");
					}else if(1 == ret){
						if(gf_tm_change_task_state(sat, orb, GF_TM_STATE_TRANSMIT, 1)){
							gf_tm_log(sat, orb, chn, NULL, "Add channel", "Change task state error");
							ret = -1;
						}
					}else if(2 == ret){
						gf_tm_close_workspace(sat, orb, NULL, NULL);
						if(gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1)){
							gf_tm_log(sat, orb, chn, NULL, "Add channel", "Change task state error");
						}
						ret = -1;
					}
					if(1 == ret) ret = 0;
				}else if(-1 == ret) gf_tm_log(sat, orb, chn, NULL, "Add channel", "Add channel to database failed");
			}else if(-1 == ret) gf_tm_log(sat, orb, chn, NULL, "Add channel", "Check channel exists failed");
		}else{
			gf_tm_log(sat, orb, chn, NULL, "Add channel", "Owner task not in POLLING");
			ret = -1;
			errno = GFSTATEERR;
		}
	}else if(0 == ret){
		gf_tm_log(sat, orb, chn, NULL, "Add channel", "Owner task not exists");
		ret = -1;
		errno = GFNEXIST;
	}else if(-1 == ret) gf_tm_log(sat, orb, chn, NULL, "Add channel", "Check task exists failed");
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, chn, NULL, "Add channel", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) GF_ERROR(errno)
	else if(0 == ret) gf_tm_log(sat, orb, chn, NULL, "Add channel", "Success");

	return 0;
}

int gf_tm_add_file(char *sat, char *orb, char *chn, char *filename, int64_t size)
{
	int  ret, iflag;
	char state[GF_TM_STATE_STR_MAX+1];
	gf_tm_sat_config_t *psat;
	gf_tm_file_t file;

	psat = gf_tm_find_sat_config(sat);
	if(!psat){
		gf_tm_log(sat, orb, chn, filename, "Add file", "Satlite not exists");
		GF_ERROR(GFNEXIST);
	}

	if(gf_tm_lock()){
		gf_tm_log(sat, orb, chn, filename, "Add file", "Lock TM failed");
		GF_ERROR(errno);
	}
	iflag = 0;
	ret = gf_tm_exist_chn(sat, orb, chn, state, NULL);
	if(1 == ret){
		if(GF_TM_ADD_ALLOW(state)){
			if(0 == gf_tm_exist_file(sat, orb, chn, filename, NULL, NULL, NULL, NULL)){
				ret = gf_tm_db_add_file(sat, orb, chn, filename, psat->priority, size);
				if(0 == ret){
					strcpy(file.state, GF_TM_STATE_TRANSMIT);
					strcpy(file.statebackup, GF_TM_STATE_NULL);
					strcpy(file.sat, sat);
					strcpy(file.orb, orb);
					strcpy(file.chn, chn);
					strcpy(file.filename, filename);
					file.priority = psat->priority;
					file.size = size;
					file.recv_size = 0;
					file.retran = 0;
					ret = gf_tm_open_workspace(&file, 0, GF_TM_TRANS_QUEUE);
					if(-1 == ret){
						GF_ERROR_LOG(errno);
						gf_tm_change_file_state(GF_TM_FILE_PARAM(file), GF_TM_STATE_ERROR, 1);
						gf_tm_log(sat, orb, chn, filename, "Add file", "Open workspace failed");
					}else{
						gf_tm_log(sat, orb, chn, filename, "Add file", "Success");
						iflag = 1;
					}
				}else if(-1 == ret){
					GF_ERROR_LOG(errno);
					gf_tm_log(sat, orb, chn, filename, "Add file", "Add file to database failed");
				}
			}
		}else{
			gf_tm_log(sat, orb, chn, filename, "Add file", "Owner channel not in POLLING");
			ret = -1;
			GF_ERROR_LOG(GFSTATEERR);
		}
	}else if(0 == ret){
		gf_tm_log(sat, orb, chn, filename, "Add file", "Owner channel not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
	}else if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, chn, filename, "Add file", "Check channel exists failed");
	}
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, chn, filename, "Add file", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	if(iflag && gf_tm_data_broadcast()) GF_ERROR(errno);

	return 0;
}

int gf_tm_pause_task(char *sat, char *orb)
{
	char state[GF_TM_STATE_STR_MAX+1];
	int ret;
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(0 == ret){
		gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Task not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
	}else if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Check task state failed");
	}else{
		if(GF_TM_PAUSE_ALLOW(state)){
			if(0 == gf_tm_change_task_state(sat, orb, GF_TM_STATE_PAUSE, 1)){
				gf_tm_pause_workspace(sat, orb, NULL, NULL);
			}else{
				GF_ERROR_LOG(errno);
				gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Set task state failed");
			}
		}else{
			gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Task not in right state");
			ret = -1;
			GF_ERROR_LOG(GFSTATEERR);
		}
	}
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	if(0 == ret) gf_tm_log(sat, orb, NULL, NULL, "Pause task", "Success");
	
	return 0;
}

int gf_tm_resume_task(char *sat, char *orb)
{
	char state[GF_TM_STATE_STR_MAX+1], statebackup[GF_TM_STATE_STR_MAX+1];
	int ret, iflag = 0;
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, statebackup);
	if(0 == ret){
		gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Task not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
	}else if(-1 == ret) gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Check task state failed");
	else{
		if(GF_TM_RESUME_ALLOW(state)){
			if(0 == gf_tm_change_task_state(sat, orb, statebackup, 1)){
				gf_tm_resume_workspace(sat, orb, NULL, NULL);
				iflag = 1;
				ret = gf_tm_check_task_done(sat, orb);
				if(-1 == ret){
					GF_ERROR_LOG(errno);
				}else if(1 == ret){
					if(gf_tm_change_task_state(sat, orb, GF_TM_STATE_HANDLE, 1)){
						GF_ERROR_LOG(errno);
						ret = -1;
					}else if(gf_tm_handle_post()){
						GF_ERROR_LOG(errno);
						ret = -1;
					}
				}
			}else{
				GF_ERROR_LOG(errno);
				gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Set task state failed");
			}
		}else{
			gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Task not in right state");
			ret = -1;
			GF_ERROR_LOG(GFSTATEERR);
		}
	}
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	if(iflag && gf_tm_data_broadcast()) GF_ERROR(errno);
	gf_tm_log(sat, orb, NULL, NULL, "Resume task", "Success");

	return 0;
}

int gf_tm_clear_workfile(char *sat, char *orb, char *chn, char *filename, char *suffix)
{
	char path[GF_TM_PATH_MAX+1];
	
	if(gf_tm_make_file_path(path, sat, orb, chn, filename, suffix)) GF_ERROR(errno);
	if(gf_tm_delete_path(path)) GF_ERROR(errno);
	
	return 0;
}

int gf_tm_cancel_task(char *sat, char *orb)
{
	char state[GF_TM_STATE_STR_MAX+1];
	int ret;
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(0 == ret){
		gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Task not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
	}else if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Check task state failed");
	}else{
		if(GF_TM_CANCEL_ALLOW(state)){
			gf_tm_close_workspace(sat, orb, NULL, NULL);
			if(strcmp(sat, gf_tm_runtime.handle_board.sat) == 0 &&\
				strcmp(orb, gf_tm_runtime.handle_board.orb) == 0){
				if(gf_tm_runtime.handle_board.pid >0) kill(gf_tm_runtime.handle_board.pid, SIGKILL);
				if(gf_tm_handle_lock()){
					GF_ERROR_LOG(errno);
					gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
					gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Lock handle failed");
					ret = -1;
				}
				if(gf_tm_handle_unlock()){
					GF_ERROR_LOG(errno);
					gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
					gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Unlock handle failed");
					ret = -1;
				}
			}
			if(gf_tm_clear_workfile(sat, orb, NULL, NULL, NULL)){
				GF_ERROR_LOG(errno);
				gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
				gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Task's workfiles delete failed");
				ret = -1;
			}else{
				if(gf_tm_db_clean_task(sat, orb)){
					GF_ERROR_LOG(errno);
					ret = -1;
					gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
					gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Clean task in database failed");
				}else{
					if(gf_tm_change_task_state(sat, orb, GF_TM_STATE_CANCEL, 1)){
						GF_ERROR_LOG(errno);
						ret = -1;
						gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Set task state failed");
					}
				}
			}
		}else{
			gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Task not in right state");
			ret = -1;
			GF_ERROR_LOG(GFSTATEERR);
		}
	}
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	gf_tm_log(sat, orb, NULL, NULL, "Cancel task", "Success");
	
	return 0;
}

int gf_tm_retran_task(char *sat, char *orb)
{
	char state[GF_TM_STATE_STR_MAX+1];
	int ret;
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(0 == ret){
		gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Task not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
	}else if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Check task state failed");
	}else{
		if(1){//GF_TM_IS_ERROR(state)||GF_TM_IS_DONE(state)||GF_TM_IS_CANCEL(state)){
			gf_tm_close_workspace(sat, orb, NULL, NULL);
			if(strcmp(sat, gf_tm_runtime.handle_board.sat) == 0 &&\
				strcmp(orb, gf_tm_runtime.handle_board.orb) == 0){
				if(gf_tm_runtime.handle_board.pid >0) kill(gf_tm_runtime.handle_board.pid, SIGKILL);
				if(gf_tm_handle_lock()){
					GF_ERROR_LOG(errno);
					gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
					gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Lock handle failed");
					ret = -1;
				}
				if(gf_tm_handle_unlock()){
					GF_ERROR_LOG(errno);
					gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
					gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Unlock handle failed");
					ret = -1;
				}
			}
			if(gf_tm_clear_workfile(sat, orb, NULL, NULL, NULL)){
				GF_ERROR_LOG(errno);
				gf_tm_change_task_state(sat, orb, GF_TM_STATE_ERROR, 1);
				gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Task's workfiles delete failed");
				ret = -1;
			}else{
				if(gf_tm_db_clean_task(sat, orb)){
					GF_ERROR_LOG(errno);
					ret = -1;
					gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Clean task in database failed");
				}else{
					if(gf_tm_add_task_retran(sat, orb)){
						GF_ERROR_LOG(errno);
						ret = -1;
						gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Add retran counter failed");
					}else if(gf_tm_change_task_state(sat, orb, GF_TM_STATE_POLL, 1)){
						GF_ERROR_LOG(errno);
						ret = -1;
						gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Set task state failed");
					}
				}
			}
		}else{
			gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Task not in ERROR,DONE,CANCEL state");
			ret = -1;
			GF_ERROR_LOG(GFSTATEERR);
		}
	}
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	gf_tm_log(sat, orb, NULL, NULL, "Retran task", "Success");
	
	return 0;
}

int gf_tm_retran_chn(char *sat, char *orb, char *chn)
{
	char state[GF_TM_STATE_STR_MAX+1];
	int ret;
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(0 == ret){
		gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Owner task not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
	}else if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Check owner task state failed");
	}else if(!GF_TM_IS_ACTIVE(state)){
		gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Owner task is not active");
		ret = -1;
		GF_ERROR_LOG(GFSTATEERR);
	}else{
		ret = gf_tm_exist_chn(sat, orb, chn, state, NULL);
		if(0 == ret){
			gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Channel not exists");
			ret = -1;
			GF_ERROR_LOG(GFNEXIST);
		}else if(-1 == ret){
			GF_ERROR_LOG(errno);
			gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Check channel state failed");
		}else{
			if(1){//GF_TM_IS_ERROR(state)){
				gf_tm_close_workspace(sat, orb, chn, NULL);
				if(strcmp(sat, gf_tm_runtime.handle_board.sat) == 0 &&\
					strcmp(orb, gf_tm_runtime.handle_board.orb) == 0 &&\
					strcmp(chn, gf_tm_runtime.handle_board.chn) == 0){
					if(gf_tm_runtime.handle_board.pid >0) kill(gf_tm_runtime.handle_board.pid, SIGKILL);
					if(gf_tm_handle_lock()){
						GF_ERROR_LOG(errno);
						gf_tm_change_chn_state(sat, orb, chn, GF_TM_STATE_ERROR, 1);
						gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Lock handle failed");
						ret = -1;
					}
					if(gf_tm_handle_unlock()){
						GF_ERROR_LOG(errno);
						gf_tm_change_chn_state(sat, orb, chn, GF_TM_STATE_ERROR, 1);
						gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Unlock handle failed");
						ret = -1;
					}
				}
				if(gf_tm_clear_workfile(sat, orb, chn, NULL, NULL)){
					GF_ERROR_LOG(errno);
					gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Channel's workfiles delete failed");
					ret = -1;
				}else{
					if(gf_tm_db_clean_chn(sat, orb, chn)){
						GF_ERROR_LOG(errno);
						ret = -1;
						gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Clean channel in database failed");
					}else{
						if(gf_tm_add_chn_retran(sat, orb, chn)){
							GF_ERROR_LOG(errno);
							ret = -1;
							gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Add retran counter failed");
						}else if(gf_tm_change_chn_state(sat, orb, chn, GF_TM_STATE_POLL, 1)){
							GF_ERROR_LOG(errno);
							ret = -1;
							gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Set channel state failed");
						}
					}
				}
			}else{
				gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Channel not in ERROR state");
				ret = -1;
				GF_ERROR_LOG(GFSTATEERR);
			}
		}
	}
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Success");
	
	return 0;
}

int gf_tm_retran_file(char *sat, char *orb, char *chn, char *filename)
{
	char state[GF_TM_STATE_STR_MAX+1];
	int ret;
	gf_tm_file_t file;
	
	if(gf_tm_lock()){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Lock TM failed");
		GF_ERROR(errno);
	}
	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(0 == ret){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Owner task not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
		goto exit_flag;
	}
	if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Check owner task state failed");
		goto exit_flag;
	}
	if(!GF_TM_IS_ACTIVE(state)){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Owner task is not active");
		ret = -1;
		GF_ERROR_LOG(GFSTATEERR);
		goto exit_flag;
	}
	ret = gf_tm_exist_chn(sat, orb, chn, state, NULL);
	if(0 == ret){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Owner channel not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
		goto exit_flag;
	}
	if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Check owner channel state failed");
		goto exit_flag;
	}
	if(!GF_TM_IS_ACTIVE(state)){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Owner channel is not active");
		ret = -1;
		GF_ERROR_LOG(GFSTATEERR);
		goto exit_flag;
	}
	ret = gf_tm_exist_file(sat, orb, chn, filename, state, NULL, &(file.priority), &(file.size));
	if(0 == ret){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "File not exists");
		ret = -1;
		GF_ERROR_LOG(GFNEXIST);
		goto exit_flag;
	}
	if(-1 == ret){
		GF_ERROR_LOG(errno);
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Check file state failed");
		goto exit_flag;
	}
	if(1){//GF_TM_IS_ERROR(state)){
		gf_tm_close_workspace(sat, orb, chn, filename);
		if(strcmp(sat, gf_tm_runtime.handle_board.sat) == 0 &&\
			strcmp(orb, gf_tm_runtime.handle_board.orb) == 0 &&\
			strcmp(chn, gf_tm_runtime.handle_board.chn) == 0 &&\
			strcmp(filename, gf_tm_runtime.handle_board.filename) == 0){
			if(gf_tm_runtime.handle_board.pid >0) kill(gf_tm_runtime.handle_board.pid, SIGKILL);
			if(gf_tm_handle_lock()){
				GF_ERROR_LOG(errno);
				gf_tm_change_file_state(sat, orb, chn, filename, GF_TM_STATE_ERROR, 1);
				gf_tm_log(sat, orb, chn, filename, "Retran file", "Lock handle failed");
				ret = -1;
			}
			if(gf_tm_handle_unlock()){
				GF_ERROR_LOG(errno);
				gf_tm_change_file_state(sat, orb, chn, filename, GF_TM_STATE_ERROR, 1);
				gf_tm_log(sat, orb, chn, filename, "Retran file", "Unlock handle failed");
				ret = -1;
			}
		}/*
		if(gf_tm_clear_workfile(sat, orb, chn, filename, NULL)){
			GF_ERROR_LOG(errno);
			gf_tm_log(sat, orb, chn, filename, "Retran file", "File's workfiles delete failed");
			ret = -1;
			goto exit_flag;
		}
		if(gf_tm_clear_workfile(sat, orb, chn, filename, GF_TM_STATUS_FILE_SUFFIX)){
			GF_ERROR_LOG(errno);
			gf_tm_log(sat, orb, chn, filename, "Retran file", "File's workfiles delete failed");
			ret = -1;
			goto exit_flag;
		}*/
		if(gf_tm_add_file_retran(sat, orb, chn, filename)){
			GF_ERROR_LOG(errno);
			ret = -1;
			gf_tm_log(sat, orb, chn, NULL, "Retran channel", "Add retran counter failed");
			goto exit_flag;
		}
		if(gf_tm_change_file_state(sat, orb, chn, filename, GF_TM_STATE_TRANSMIT, 1)){
			GF_ERROR_LOG(errno);
			ret = -1;
			gf_tm_log(sat, orb, chn, filename, "Retran file", "Clean file in database failed");
			goto exit_flag;
		}
		strcpy(file.state, GF_TM_STATE_TRANSMIT);
		strcpy(file.statebackup, GF_TM_STATE_NULL);
		strcpy(file.sat, sat);
		strcpy(file.orb, orb);
		strcpy(file.chn, chn);
		strcpy(file.filename, filename);
		file.recv_size = 0;
		if(gf_tm_open_workspace(&file, 0, GF_TM_TRANS_QUEUE)){
			GF_ERROR_LOG(errno);
			ret = -1;
			gf_tm_change_file_state(sat, orb, chn, filename, GF_TM_STATE_ERROR, 1);
			gf_tm_log(sat, orb, chn, filename, "Retran file", "Open file workspace failed");
			goto exit_flag;
		}
		if(gf_tm_data_broadcast()) GF_ERROR(errno);
	}else{
		gf_tm_log(sat, orb, chn, filename, "Retran file", "file not in ERROR state");
		ret = -1;
		GF_ERROR_LOG(GFSTATEERR);
	}
exit_flag:
	if(gf_tm_unlock()){
		gf_tm_log(sat, orb, chn, filename, "Retran file", "Unlock TM failed");
		GF_ERROR(errno);
	}
	if(-1 == ret) return -1;
	gf_tm_log(sat, orb, chn, filename, "Retran file", "Success");
	
	return 0;
}

/************DEBUG***********/
void gf_tm_print_workspace(gf_tm_workspace_t *p, int showblock)
{
	printf("[Workspace]\n");
	gf_tm_print_file(&(p->file));
	printf("DataPath: %s\n", p->datapath);
	printf("StatusPath: %s\n", p->statuspath);
	printf("bsize:%ld bcount:%ld datafd:%d checkfd:%d alloc_index:%ld chk:%d comp:%d\ndone_count:%ld doing_count:%ld error_count:%ld\n",\
		p->block_size, p->block_count, p->datafd, p->statusfd, p->alloc_index,\
		p->check, p->compress, p->done_count, p->doing_count, p->error_count);
	if(showblock){
		int64_t i;
		for(i = 0; i < p->block_count; i++){
			printf("%02X ", p->block_state[i]);
			if(i % 32 == 31) printf("\n");
		}
		if(i % 32 != 0) printf("\n");
	}
}

void gf_tm_print_workspace_queue(void)
{
	gf_tm_workspace_t *p;

	p = gf_tm_trans_queue.next;
	printf("<TransQueue>\n");
	while(p){
		gf_tm_print_workspace(p, 1);
		p = p->next;
	}
	p = gf_tm_pause_queue.next;
	printf("<PauseQueue>\n");
	while(p){
		gf_tm_print_workspace(p, 1);
		p = p->next;
	}
}

void gf_tm_print_workslot(gf_tm_work_t *p)
{
	gf_tm_print_file(&(p->file));
	printf("state:%d index:%ld\n", p->state, p->block_index);
}

void gf_tm_print_workslots(void)
{
	int i;

	for(i = 0; i < gf_tm_config.thread_num; i++){
		printf("<Workslot %d>\n", i);
		gf_tm_print_workslot(&gf_tm_work_slot[i]);
	}
}
