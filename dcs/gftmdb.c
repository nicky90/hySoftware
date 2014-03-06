/*
 * gftmdb.h - GF task manager database
 *
 * These functions maybe not threads safe
 *
 * Writen by CX3201 2013-06-11
 */

#include "gftmdb.h"
#include "gferror.h"
#include <string.h>

int gf_tm_init_tables(void)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR_PR(GFALLOCERR);
	gf_db_query_with_reconn(&hstmt, "DROP TABLE gf_state");
	gf_db_query_with_reconn(&hstmt, "DROP TABLE gf_debug");
	gf_db_query_with_reconn(&hstmt, "DROP TABLE gf_log");
	gf_db_query_with_reconn(&hstmt, "DROP TABLE gf_file");
	gf_db_query_with_reconn(&hstmt, "DROP TABLE gf_chn");
	gf_db_query_with_reconn(&hstmt, "DROP TABLE gf_task");

	sprintf(sql, "CREATE TABLE gf_task"\
			"(state CHAR(%d),"\
			"sat VARCHAR(%d),"\
			"orb VARCHAR(%d),"\
			"statebackup CHAR(%d),"\
			"chnnum INT,"\
			"retran INT,"\
			"addtime DATETIME,"\
			"donetime DATETIME,"\
			"PRIMARY KEY(sat,orb)) type=innoDB",
			GF_TM_STATE_STR_MAX,
			GF_TM_SAT_NAME_MAX,
			GF_TM_ORB_NAME_MAX,
			GF_TM_STATE_STR_MAX);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "CREATE TABLE gf_chn"\
			"(state CHAR(%d),"\
			"sat VARCHAR(%d),"\
			"orb VARCHAR(%d),"\
			"chn VARCHAR(%d),"\
			"statebackup CHAR(%d),"\
			"retran INT,"\
			"addtime DATETIME,"\
			"PRIMARY KEY(sat,orb,chn)) type=innoDB",
			GF_TM_STATE_STR_MAX,
			GF_TM_SAT_NAME_MAX,
			GF_TM_ORB_NAME_MAX,
			GF_TM_CHN_NAME_MAX,
			GF_TM_STATE_STR_MAX);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "CREATE TABLE gf_file"\
			"(state CHAR(%d),"\
			"sat VARCHAR(%d),"\
			"orb VARCHAR(%d),"\
			"chn VARCHAR(%d),"\
			"filename VARCHAR(%d),"\
			"statebackup CHAR(%d),"\
			"priority INT,"\
			"size BIGINT,"\
			"recv_size BIGINT,"\
			"retran INT,"\
			"addtime DATETIME,"\
			"PRIMARY KEY(sat,orb,chn,filename)) type=innoDB",
			GF_TM_STATE_STR_MAX,
			GF_TM_SAT_NAME_MAX,
			GF_TM_ORB_NAME_MAX,
			GF_TM_CHN_NAME_MAX,
			GF_TM_FILE_NAME_MAX,
			GF_TM_STATE_STR_MAX);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "CREATE TABLE gf_log"\
			"(time DATETIME,"\
			"sat VARCHAR(%d),"\
			"orb VARCHAR(%d),"\
			"chn VARCHAR(%d),"\
			"filename VARCHAR(%d),"\
			"op VARCHAR(%d),"\
			"msg VARCHAR(%d)) type=innoDB",
			GF_TM_SAT_NAME_MAX,
			GF_TM_ORB_NAME_MAX,
			GF_TM_CHN_NAME_MAX,
			GF_TM_FILE_NAME_MAX,
			GF_TM_OP_STR_MAX,
			GF_TM_LOGMSG_MAX);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "CREATE TABLE gf_debug"\
			"(time DATETIME,"\
			"code INT,"\
			"file VARCHAR(%d),"\
			"func VARCHAR(%d),"\
			"line INT,"\
			"msg VARCHAR(%d)) type=innoDB",
			GF_TM_FILE_NAME_MAX,
			GF_TM_FILE_NAME_MAX,
			GF_TM_LOGMSG_MAX);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "CREATE TABLE gf_state"\
			"(statename CHAR(40),starttime DATETIME,activetime TIMESTAMP,"\
			"state CHAR(%d),"\
			"speed BIGINT) type=innoDB",
			GF_TM_STATE_STR_MAX);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "INSERT INTO gf_state(statename,starttime,state,speed) VALUES"\
			"('%s',NOW(),"\
			"NULL,"\
			"0)","speed");
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "INSERT INTO gf_state(statename,starttime,state,speed) VALUES"\
			"('%s',NOW(),"\
			"NULL,"\
			"0)","GF01");
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "INSERT INTO gf_state(statename,starttime,state,speed) VALUES"\
			"('%s',NOW(),"\
			"NULL,"\
			"0)","GF02");
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	sprintf(sql, "INSERT INTO gf_state(statename,starttime,state,speed) VALUES"\
			"('%s',NOW(),"\
			"NULL,"\
			"0)","GF03");
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}
	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_log(char *sat, char *orbit, char *chn, char *filename, char *op, char *msg)
{
	char      sql[GF_TM_SQL_MAX];
	char      temp[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!msg || !op) GF_ERROR_PR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR_PR(GFALLOCERR);
	sprintf(sql, "INSERT INTO gf_log(time,sat,orb,chn,filename,op,msg) VALUES(NOW()");
	if(sat){
		sprintf(temp, ",\'%s\'", sat);
		strcat(sql, temp);
	}else strcat(sql, ",NULL");
	if(orbit){
		sprintf(temp, ",\'%s\'", orbit);
		strcat(sql, temp);
	}else strcat(sql, ",NULL");
	if(chn){
		sprintf(temp, ",\'%s\'", chn);
		strcat(sql, temp);
	}else strcat(sql, ",NULL");
	if(filename){
		sprintf(temp, ",\'%s\'", filename);
		strcat(sql, temp);
	}else strcat(sql, ",NULL");
	sprintf(temp, ",\'%s\',\'%s\')", op, msg);
	strcat(sql, temp);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_debug(int code, const char *file, const char *func, int line, char *msg)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!msg) GF_ERROR_PR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR_PR(GFALLOCERR);

	sprintf(sql, "INSERT INTO gf_debug(time,code,file,func,line,msg) VALUES(NOW()"\
			",%d"\
			",\'%s\'"\
			",\'%s\'"\
			",%d"\
			",\'%s\')",
			code,file,func,line,msg);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_refresh_starttime(void)
{
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	ret = gf_db_query_with_reconn(&hstmt, "UPDATE gf_state SET starttime=NOW(),activetime=NOW() where statename='speed'");
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_refresh_state(char *state)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_state SET state=\'%s\',activetime=NOW() where statename='speed'", state);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_refresh_speed(int64_t speed)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_state SET speed=%lld,activetime=NOW() where statename='speed'", (long long int)speed);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_delete_link()
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "delete from gf_state where statename like 'GF%%'");
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_insert_link(char *staname)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);
	sprintf(sql, "INSERT INTO gf_state(statename,starttime,state,speed) VALUES"\
                        "('%s',NOW(),"\
                        "'OFF',"\
                        "0)",staname);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_refresh_link(char *staname)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_state SET speed=1,state='ON',activetime=NOW() where statename='%s'", staname);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_refresh_unlink(char *staname)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_state SET speed=0,state='OFF',activetime=NOW() where statename='%s'", staname);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}


int gf_tm_refresh_task_donetime(char *sat, char *orb)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_task SET donetime=NOW() where sat=\'%s\' AND orb=\'%s\'", sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

/* 0-not exist 1-success -1-error */
int gf_tm_get_handle_task(gf_tm_task_t *ptask)
{
	SQLRETURN sret;
	SQLHSTMT  hstmt = NULL;
	int tmp;

	if(!ptask) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(&hstmt, "SELECT * from gf_task where state=\'"GF_TM_STATE_HANDLE"\' limit 1")){
		tmp = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(tmp);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		gf_db_free_stmt(hstmt);
		return 0;
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(gf_tm_get_task(hstmt, ptask)){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}
	}
	gf_db_free_stmt(hstmt);
	return 1;
}

int gf_tm_query_active_task(SQLHSTMT *phstmt)
{
	int tmp;

	if(!phstmt) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(phstmt);
	if(!*phstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(phstmt, "SELECT * from gf_task where state=\'"GF_TM_STATE_POLL"\'")){
		tmp = errno;
		gf_db_free_stmt(*phstmt);
		GF_ERROR(tmp);
	}
	return 0;
}

int gf_tm_get_task(SQLHSTMT hstmt, gf_tm_task_t *ptask)
{
	if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_CHAR, ptask->state, GF_TM_STATE_STR_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 2, SQL_C_CHAR, ptask->sat, GF_TM_SAT_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 3, SQL_C_CHAR, ptask->orb, GF_TM_ORB_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 4, SQL_C_CHAR, ptask->statebackup, GF_TM_STATE_STR_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 5, SQL_C_SLONG, &(ptask->chnnum), 0, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 6, SQL_C_SLONG, &(ptask->retran), 0, NULL)))
		GF_ERROR(GFDATAERR);

	return 0;
}

/* 0-not exist 1-success -1-error */
int gf_tm_get_handle_chn(gf_tm_chn_t *pchn)
{
	SQLRETURN sret;
	SQLHSTMT  hstmt = NULL;
	int tmp;

	if(!pchn) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(&hstmt, "SELECT * from gf_chn where state=\'"GF_TM_STATE_HANDLE"\' limit 1")){
		tmp = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(tmp);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		gf_db_free_stmt(hstmt);
		return 0;
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(gf_tm_get_chn(hstmt, pchn)){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}
	}
	gf_db_free_stmt(hstmt);
	return 1;
}

int gf_tm_query_active_chn(SQLHSTMT *phstmt)
{
	int tmp;

	if(!phstmt) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(phstmt);
	if(!*phstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(phstmt, "SELECT gf_chn.* from gf_task,gf_chn "\
		"where (gf_task.state=\'"GF_TM_STATE_POLL"\' OR gf_task.state=\'"GF_TM_STATE_TRANSMIT"\') AND "\
		"(gf_task.sat=gf_chn.sat AND gf_task.orb=gf_chn.orb) AND "\
		"gf_chn.state=\'"GF_TM_STATE_POLL"\'")){
		tmp = errno;
		gf_db_free_stmt(*phstmt);
		GF_ERROR(tmp);
	}
	return 0;
}

int gf_tm_get_chn(SQLHSTMT hstmt, gf_tm_chn_t *pchn)
{
	if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_CHAR, pchn->state, GF_TM_STATE_STR_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 2, SQL_C_CHAR, pchn->sat, GF_TM_SAT_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 3, SQL_C_CHAR, pchn->orb, GF_TM_ORB_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 4, SQL_C_CHAR, pchn->chn, GF_TM_CHN_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 5, SQL_C_CHAR, pchn->statebackup, GF_TM_STATE_STR_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 6, SQL_C_SLONG, &(pchn->retran), 0, NULL)))
		GF_ERROR(GFDATAERR);

	return 0;
}

/* 0-not exist 1-success -1-error */
int gf_tm_get_handle_file(gf_tm_file_t *pfile)
{
	SQLRETURN sret;
	SQLHSTMT  hstmt = NULL;
	int tmp;

	if(!pfile) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(&hstmt, "SELECT * from gf_file where state=\'"GF_TM_STATE_HANDLE"\' limit 1")){
		tmp = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(tmp);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		gf_db_free_stmt(hstmt);
		return 0;
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(gf_tm_get_file(hstmt, pfile)){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}
	}
	gf_db_free_stmt(hstmt);
	return 1;
}

int gf_tm_query_trans_file(SQLHSTMT *phstmt)
{
	int tmp;

	if(!phstmt) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(phstmt);
	if(!*phstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(phstmt, "SELECT gf_file.* from gf_file,gf_task where "\
		"(gf_task.state=\'"GF_TM_STATE_POLL"\' OR gf_task.state=\'"GF_TM_STATE_TRANSMIT"\') AND "\
		"gf_file.sat=gf_task.sat AND gf_file.orb=gf_task.orb AND "\
		"gf_file.state=\'"GF_TM_STATE_TRANSMIT"\'")){
		tmp = errno;
		gf_db_free_stmt(*phstmt);
		GF_ERROR(tmp);
	}	
	return 0;
}

int gf_tm_query_pause_file(SQLHSTMT *phstmt)
{
	int tmp;

	if(!phstmt) GF_ERROR(GFNULLPARAM);
	gf_db_get_stmt(phstmt);
	if(!*phstmt) GF_ERROR(GFALLOCERR);

	if(gf_db_query_with_reconn(phstmt, "SELECT gf_file.* from gf_file,gf_task where "\
		"gf_task.state=\'"GF_TM_STATE_PAUSE"\' AND "\
		"gf_file.sat=gf_task.sat AND gf_file.orb=gf_task.orb AND "\
		"gf_file.state=\'"GF_TM_STATE_TRANSMIT"\'")){
		tmp = errno;
		gf_db_free_stmt(*phstmt);
		GF_ERROR(tmp);
	}
	return 0;
}

int gf_tm_get_file(SQLHSTMT hstmt, gf_tm_file_t *pfile)
{
	if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_CHAR, pfile->state, GF_TM_STATE_STR_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 2, SQL_C_CHAR, pfile->sat, GF_TM_SAT_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 3, SQL_C_CHAR, pfile->orb, GF_TM_ORB_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 4, SQL_C_CHAR, pfile->chn, GF_TM_CHN_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 5, SQL_C_CHAR, pfile->filename, GF_TM_FILE_NAME_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 6, SQL_C_CHAR, pfile->statebackup, GF_TM_STATE_STR_MAX+1, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 7, SQL_C_SLONG, &(pfile->priority), 0, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 8, SQL_C_SBIGINT, &(pfile->size), 0, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 9, SQL_C_SBIGINT, &(pfile->recv_size), 0, NULL)))
		GF_ERROR(GFDATAERR);
	if(!SQL_OK(SQLGetData(hstmt, 10, SQL_C_SLONG, &(pfile->retran), 0, NULL)))
		GF_ERROR(GFDATAERR);

	return 0;
}

int gf_tm_change_file_state(char *sat, char *orb, char *chn, char *filename, char *state, int backup)
{
	char      sql[GF_TM_SQL_MAX];
	char      temp[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!state) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);
	if(backup) sprintf(sql, "UPDATE gf_file SET statebackup=state, state=\'%s\' where 1", state);
	else sprintf(sql, "UPDATE gf_file SET state=\'%s\' where 1", state);
	if(sat){
		sprintf(temp, " AND sat=\'%s\'", sat);
		strcat(sql, temp);
	}
	if(orb){
		sprintf(temp, " AND orb=\'%s\'", orb);
		strcat(sql, temp);
	}
	if(chn){
		sprintf(temp, " AND chn=\'%s\'", chn);
		strcat(sql, temp);
	}
	if(filename){
		sprintf(temp, " AND filename=\'%s\'", filename);
		strcat(sql, temp);
	}

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_set_file_recv_size(char *sat, char *orb, char *chn, char *filename, int64_t recv_size)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn || !filename) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_file SET recv_size=%lld "\
		"where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\' AND filename=\'%s\'",
		(long long int)recv_size, sat, orb, chn, filename);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	gf_db_free_stmt(hstmt);

	return 0;
}

/* 0-not exist 1-exist -1-error */
int gf_tm_exist_task(char *sat, char *orb, char *state, char *statebackup)
{
	SQLRETURN sret;
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT state,statebackup from gf_task where sat=\'%s\' AND orb=\'%s\' limit 1",
			sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret) ret = 0;
	else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(state){
			if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_CHAR, state, GF_TM_STATE_STR_MAX+1, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		if(statebackup){
			if(!SQL_OK(SQLGetData(hstmt, 2, SQL_C_CHAR, statebackup, GF_TM_STATE_STR_MAX+1, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		ret = 1;
	}

	gf_db_free_stmt(hstmt);

	return ret;
}

/* 0-not exist 1-exist -1-error */
int gf_tm_exist_chn(char *sat, char *orb, char *chn, char *state, char *statebackup)
{
	SQLRETURN sret;
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT state,statebackup from gf_chn where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\' limit 1",
			sat, orb, chn);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret) ret = 0;
	else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(state){
			if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_CHAR, state, GF_TM_STATE_STR_MAX+1, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		if(statebackup){
			if(!SQL_OK(SQLGetData(hstmt, 2, SQL_C_CHAR, statebackup, GF_TM_STATE_STR_MAX+1, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		ret = 1;
	}

	gf_db_free_stmt(hstmt);

	return ret;
}

/* 0-not exist 1-exist -1-error */
int gf_tm_exist_file(char *sat, char *orb, char *chn, char *filename, char *state, char *statebackup, int *pPriority, int64_t *pSize)
{
	SQLRETURN sret;
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn || !filename) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT state,statebackup,priority,size from gf_file where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\' AND filename=\'%s\' limit 1",
			sat, orb, chn, filename);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret) ret = 0;
	else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(state){
			if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_CHAR, state, GF_TM_STATE_STR_MAX+1, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		if(statebackup){
			if(!SQL_OK(SQLGetData(hstmt, 2, SQL_C_CHAR, statebackup, GF_TM_STATE_STR_MAX+1, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		if(pPriority){
			if(!SQL_OK(SQLGetData(hstmt, 3, SQL_C_SLONG, pPriority, 0, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		if(pSize){
			if(!SQL_OK(SQLGetData(hstmt, 4, SQL_C_SBIGINT, pSize, 0, NULL))){
				gf_db_free_stmt(hstmt);
				GF_ERROR(GFDATAERR);
			}
		}
		ret = 1;
	}

	gf_db_free_stmt(hstmt);

	return ret;
}

int gf_tm_db_add_task(char *sat, char *orb, int chnnum)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);
	if(chnnum < 0) GF_ERROR(GFINVAPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "INSERT INTO gf_task VALUES(\'"GF_TM_STATE_POLL"\'"\
			",\'%s\'"\
			",\'%s\',\'"GF_TM_STATE_NULL"\'"\
			",%d,0,NOW(),NULL)",
			sat, orb, chnnum);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_db_add_chn(char *sat, char *orb, char *chn)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "INSERT INTO gf_chn VALUES(\'"GF_TM_STATE_POLL"\'"\
			",\'%s\'"\
			",\'%s\'"\
			",\'%s\',\'"GF_TM_STATE_NULL"\',0,NOW())",
			sat, orb, chn);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_db_add_file(char *sat, char *orb, char *chn, char *filename, int priority, int64_t size)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn || !filename) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "INSERT INTO gf_file VALUES(\'"GF_TM_STATE_TRANSMIT"\'"\
			",\'%s\'"\
			",\'%s\'"\
			",\'%s\'"\
			",\'%s\'"\
			",\'"GF_TM_STATE_NULL"\',%d,%lld,0,0,NOW())",
			sat, orb, chn, filename, priority, (long long int)size);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

/* 0-poll 1-complete 2-task error -1-error */
int gf_tm_check_poll_task(char *sat, char *orb)
{
	SQLRETURN sret;
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret, chnnum, count;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT chnnum from gf_task where sat=\'%s\' AND orb=\'%s\' limit 1",
			sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFNEXIST);
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_SLONG, &chnnum, 0, NULL))){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}
	}
	gf_db_free_stmt(hstmt);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT COUNT(*) from gf_chn where sat=\'%s\' AND orb=\'%s\'",
			sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFNEXIST);
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}else{
		if(!SQL_OK(SQLGetData(hstmt, 1, SQL_C_SLONG, &count, 0, NULL))){
			gf_db_free_stmt(hstmt);
			GF_ERROR(GFDATAERR);
		}
	}
	gf_db_free_stmt(hstmt);

	if(count == chnnum){
		return 1;
	}else if(count > chnnum){
		return 2;
	}

	return 0;
}

int gf_tm_change_task_state(char *sat, char *orb, char *state, int backup)
{
	char      sql[GF_TM_SQL_MAX];
	char      temp[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!state) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);
	if(backup) sprintf(sql, "UPDATE gf_task SET statebackup=state, state=\'%s\' where 1", state);
	else sprintf(sql, "UPDATE gf_task SET state=\'%s\' where 1", state);
	if(sat){
		sprintf(temp, " AND sat=\'%s\'", sat);
		strcat(sql, temp);
	}
	if(orb){
		sprintf(temp, " AND orb=\'%s\'", orb);
		strcat(sql, temp);
	}

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_change_chn_state(char *sat, char *orb, char *chn, char *state, int backup)
{
	char      sql[GF_TM_SQL_MAX];
	char      temp[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!state) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);
	if(backup) sprintf(sql, "UPDATE gf_chn SET statebackup=state, state=\'%s\' where 1", state);
	else sprintf(sql, "UPDATE gf_chn SET state=\'%s\' where 1", state);
	if(sat){
		sprintf(temp, " AND sat=\'%s\'", sat);
		strcat(sql, temp);
	}
	if(orb){
		sprintf(temp, " AND orb=\'%s\'", orb);
		strcat(sql, temp);
	}
	if(chn){
		sprintf(temp, " AND chn=\'%s\'", chn);
		strcat(sql, temp);
	}

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_db_clean_task(char *sat, char *orb)
{
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "DELETE from gf_chn where sat=\'%s\' AND orb=\'%s\'",
			sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	sprintf(sql, "DELETE from gf_file where sat=\'%s\' AND orb=\'%s\'",
			sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return ret;
}

int gf_tm_db_clean_chn(char *sat, char *orb, char *chn)
{
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "DELETE from gf_file where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\'",
			sat, orb, chn);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return ret;
}

int gf_tm_db_del_file(char *sat, char *orb, char *chn, char *filename)
{
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn || !filename) GF_ERROR(GFNULLPARAM);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "DELETE from gf_file where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\' AND filename=\'%s\'",
			sat, orb, chn, filename);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}

	gf_db_free_stmt(hstmt);

	return ret;
}

int gf_tm_add_task_retran(char *sat, char *orb)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);

	ret = gf_tm_exist_task(sat, orb, NULL, NULL);
	if(0 == ret) GF_ERROR(GFNEXIST);
	if(ret < 0) GF_ERROR(errno);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE "GF_TM_TASK_TABLE" SET retran=retran+1 "\
		"where sat=\'%s\' AND orb=\'%s\'", sat, orb);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_add_chn_retran(char *sat, char *orb, char *chn)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn) GF_ERROR(GFNULLPARAM);

	ret = gf_tm_exist_chn(sat, orb, chn, NULL, NULL);
	if(0 == ret) GF_ERROR(GFNEXIST);
	if(ret < 0) GF_ERROR(errno);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE "GF_TM_CHN_TABLE" SET retran=retran+1 "\
		"where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\'", sat, orb, chn);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	gf_db_free_stmt(hstmt);

	return 0;
}

int gf_tm_add_file_retran(char *sat, char *orb, char *chn, char *filename)
{
	char      sql[GF_TM_SQL_MAX];
	SQLHSTMT  hstmt = NULL;
	int       ret;

	if(!sat || !orb || !chn || !filename) GF_ERROR(GFNULLPARAM);

	ret = gf_tm_exist_file(sat, orb, chn, filename, NULL, NULL, NULL, NULL);
	if(0 == ret) GF_ERROR(GFNEXIST);
	if(ret < 0) GF_ERROR(errno);

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE "GF_TM_FILE_TABLE" SET retran=retran+1 "\
		"where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\' AND filename=\'%s\'", sat, orb, chn, filename);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	gf_db_free_stmt(hstmt);

	return 0;
}


/* 0-not done 1-done -1-error */
int gf_tm_check_task_done(char *sat, char *orb)
{
	SQLRETURN sret;
	char      sql[GF_TM_SQL_MAX+1], state[GF_TM_STATE_STR_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret = 0;

	if(!sat || !orb) GF_ERROR(GFNULLPARAM);

	ret = gf_tm_exist_task(sat, orb, state, NULL);
	if(-1 == ret) GF_ERROR(errno);
	if(0 == ret) return 0;
	if(!GF_TM_IS_TRANSMIT(state)) return 0;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT state from gf_chn where sat=\'%s\' AND orb=\'%s\' "\
			"AND state <> 'DONE' limit 1", sat, orb);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		ret = 1;
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}
	gf_db_free_stmt(hstmt);

	return ret;
}

/* 0-not done 1-done -1-error */
int gf_tm_check_chn_done(char *sat, char *orb, char *chn)
{
	SQLRETURN sret;
	char      sql[GF_TM_SQL_MAX+1], state[GF_TM_STATE_STR_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret = 0;

	if(!sat || !orb || !chn) GF_ERROR(GFNULLPARAM);

	ret = gf_tm_exist_chn(sat, orb, chn, state, NULL);
	if(-1 == ret) GF_ERROR(errno);
	if(0 == ret) return 0;
	if(!GF_TM_IS_TRANSMIT(state)) return 0;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "SELECT state from gf_file where sat=\'%s\' AND orb=\'%s\' AND chn=\'%s\' "\
			"AND state <> 'DONE' limit 1",
			sat, orb, chn);

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	sret = SQLFetch(hstmt);
	if(SQL_NO_DATA == sret){
		ret = 1;
	}else if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR(GFDATAERR);
	}
	gf_db_free_stmt(hstmt);

	return ret;
}

int gf_tm_recover_state(void)
{
	char      sql[GF_TM_SQL_MAX+1];
	SQLHSTMT  hstmt = NULL;
	int       ret = 0;

	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR(GFALLOCERR);

	sprintf(sql, "UPDATE gf_task set state=\'"GF_TM_STATE_ERROR"\' where state=\'"GF_TM_STATE_HANDLE"\'");

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	
	sprintf(sql, "UPDATE gf_chn set state=\'"GF_TM_STATE_ERROR"\' where state=\'"GF_TM_STATE_HANDLE"\'");

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	
	sprintf(sql, "UPDATE gf_file set state=\'"GF_TM_STATE_ERROR"\' where state=\'"GF_TM_STATE_HANDLE"\'");

	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret){
		ret = errno;
		gf_db_free_stmt(hstmt);
		GF_ERROR(ret);
	}
	
	gf_db_free_stmt(hstmt);

	return ret;
}
