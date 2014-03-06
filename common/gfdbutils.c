/*
 * gferror.c - database utils
 *
 * Writen by CX3201 2013-05-31
 */

#include "gfdbutils.h"
#include "gferror.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>

static gf_db_t         gf_db; /* Static global database description struct */
/* For multi-threads mutex */
static pthread_mutex_t gf_db_lock = PTHREAD_MUTEX_INITIALIZER;

/**************************************************
* Func: Initialize database operation
* dsn : Data source name
* user: User name, NULL for ""
* psw : Password, NULL for ""
* ret : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_init(char *dsn, char *user, char *psw)
{
	int       len, t;

	pthread_mutex_lock(&gf_db_lock);

	if(!dsn) GF_ERROR_PR(GFNULLPARAM);
	len = strlen(dsn);
	if(user){
		t = strlen(user);
		if(len < t) len = t;
	}
	if(psw){
		t = strlen(psw);
		if(len < t) len = t;
	}
	if(len >= GF_DB_NAME_LEN) GF_ERROR_PR(GFLONGPARAM);
	strcpy(gf_db.dsn, dsn);
	if(user) strcpy(gf_db.user, user);
	else gf_db.user[0] = '\0';
	if(psw) strcpy(gf_db.psw, psw);
	else gf_db.psw[0] = '\0';

	gf_db.henv   = NULL;
	gf_db.hdbc   = NULL;
	gf_db.isconn = 0;
	gf_db.isinit = 1;

	pthread_mutex_unlock(&gf_db_lock);

	return 0;
}

/**************************************************
* Func: Connect(re-connect) to database
* ret : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_conn(void)
{
	SQLRETURN sret;

	pthread_mutex_lock(&gf_db_lock);

	if(!gf_db.isinit || gf_db.isconn) GF_ERROR_PR(GFSTATEERR);

	sret = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&(gf_db.henv));
	if(!SQL_OK(sret)) GF_ERROR_PR(GFALLOCERR);
	sret = SQLSetEnvAttr(gf_db.henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 
	if(!SQL_OK(sret)){
		SQLFreeHandle(SQL_HANDLE_ENV, gf_db.henv);
		GF_ERROR_PR(GFSETERR);
	}
	sret = SQLAllocHandle(SQL_HANDLE_DBC, gf_db.henv, &(gf_db.hdbc));
	if(!SQL_OK(sret)){
		SQLFreeHandle(SQL_HANDLE_ENV, gf_db.henv);
		GF_ERROR_PR(GFALLOCERR);
	}
	sret = SQLSetConnectAttr(gf_db.hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)GF_DB_CONN_TIMEOUT, 0);
	if(!SQL_OK(sret)){
		SQLFreeHandle(SQL_HANDLE_ENV, gf_db.henv);
		GF_ERROR_PR(GFSETERR);
	}
	sret = SQLConnect(gf_db.hdbc, (SQLCHAR*)gf_db.dsn, SQL_NTS, (SQLCHAR*)gf_db.user, SQL_NTS, (SQLCHAR*)gf_db.psw, SQL_NTS);
	if(!SQL_OK(sret)){
		SQLFreeHandle(SQL_HANDLE_ENV, gf_db.hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, gf_db.henv);
		GF_ERROR_PR(GFCONNERR);
	}
	gf_db.isconn = 1;

	pthread_mutex_unlock(&gf_db_lock);

	return 0;
}

/**************************************************
* Func: Disonnect database
* ret : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_disconn(void)
{
	pthread_mutex_lock(&gf_db_lock);

	if(!gf_db.isinit || !gf_db.isconn) GF_ERROR_PR(GFSTATEERR);
	SQLDisconnect(gf_db.hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, gf_db.hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, gf_db.henv);
	gf_db.isconn = 0;

	pthread_mutex_unlock(&gf_db_lock);

	return 0;
}

/**************************************************
* Func: Check connected or not
* ret : 0 not connect, 1 already connected
**************************************************/
int gf_db_is_conn(void)
{
	return gf_db.isconn;
}

/**************************************************
* Func   : Get a stmt to query
* phstmt : Pointer to hstmt
* ret    : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_get_stmt(SQLHSTMT *phstmt)
{
	SQLRETURN sret;

	if(!gf_db.isinit || !gf_db.isconn) GF_ERROR_PR(GFSTATEERR);
	if(!phstmt) GF_ERROR_PR(GFNULLPARAM);
	sret = SQLAllocHandle(SQL_HANDLE_STMT, gf_db.hdbc, phstmt);
	if(!SQL_OK(sret)) GF_ERROR_PR(GFALLOCERR);

	return 0;
}

/**************************************************
* Func : Get a stmt to query
* hstmt: Hstmt
* ret  : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_free_stmt(SQLHSTMT hstmt)
{
	if(!hstmt) GF_ERROR_PR(GFNULLPARAM);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	return 0;
}

/**************************************************
* Func  : Query SQL statement, reconnect when lose
* phstmt: Pointer of hstmt
* sql   : SQL statement
* ret   : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_query_with_reconn(SQLHSTMT *phstmt, char * sql)
{
	int        ret;
	SQLRETURN  sret;
	SQLCHAR    sql_state[6];
	SQLINTEGER sql_code;
	

	if(!phstmt || !sql) GF_ERROR_PR(GFNULLPARAM);
	sret = SQLExecDirect(*phstmt, (SQLCHAR*)sql, SQL_NTS);
	if(!SQL_OK(sret)){
		sret = SQLGetDiagRec(SQL_HANDLE_STMT, *phstmt, 1, sql_state, &sql_code, NULL, 0, NULL);
		if(SQL_OK(sret) && strcmp((char*)sql_state, "08S01") != 0){ /* Not Link down */
			printf("%s\n", sql);
			GF_ERROR_PR(GFEXECERR);
		}
		gf_db_free_stmt(*phstmt);
		gf_db_disconn();
		ret = gf_db_conn();
		if(ret) GF_ERROR_PR(errno);
		ret = gf_db_get_stmt(phstmt);
		if(ret) GF_ERROR_PR(errno);
		sret = SQLExecDirect(*phstmt, (SQLCHAR*)sql, SQL_NTS);
		if(!SQL_OK(sret)) GF_ERROR_PR(GFEXECERR);
	}

	return 0;
}

/**************************************************
* Func : Test connection and reconnect when lose
* ret  : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_ping(void)
{
	int       ret;
	SQLHSTMT  hstmt = NULL;

	if(!gf_db.isinit || !gf_db.isconn) GF_ERROR_PR(GFSTATEERR);
	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR_PR(GFALLOCERR);
	ret = gf_db_query_with_reconn(&hstmt, "SELECT NOW()");
	gf_db_free_stmt(hstmt);

	if(ret) GF_ERROR_PR(errno);
	
	return 0;
}

/**************************************************
* Func: Test query SQL, print result
* sql : SQL statement
* ret : 0 success, -1 error, error code in errno
**************************************************/
int gf_db_query_test(char *sql)
{
	int         ret, i, count;
	SQLRETURN   sret;
	SQLHSTMT    hstmt = NULL;
	SQLSMALLINT ncol, slen;
	SQLLEN      dlen;
	char        col_name[GF_DB_STR_LEN], format[256];

	if(!sql) GF_ERROR_PR(GFNULLPARAM);
	if(!gf_db.isinit || !gf_db.isconn) GF_ERROR_PR(GFSTATEERR);
	gf_db_get_stmt(&hstmt);
	if(!hstmt) GF_ERROR_PR(GFALLOCERR);
	ret = gf_db_query_with_reconn(&hstmt, sql);
	if(ret) GF_ERROR_PR(errno);
	/* Get title */
	sret = SQLNumResultCols(hstmt, &ncol);
	if(!SQL_OK(sret)){
		gf_db_free_stmt(hstmt);
		GF_ERROR_PR(GFEXECERR);
	}
	for(i = 1; i <= ncol; i++){
		SQLColAttribute(hstmt, i, SQL_DESC_NAME, col_name, GF_DB_STR_LEN, &slen, NULL);
		SQLColAttribute(hstmt, i, SQL_DESC_LENGTH, NULL, 0, NULL, &dlen);
		sprintf(format, "%%-%ds", (int)dlen + 1);
		printf(format, col_name, (int)dlen);
	}
	printf("\n");
	/* Get date */
	count = 0;
	while(1){
		sret = SQLFetch(hstmt);
		if(!SQL_OK(sret)) break;
		for(i = 1; i <= ncol; i++){
			/* Get len */
			SQLColAttribute(hstmt, i, SQL_DESC_LENGTH, NULL, 0, NULL, &dlen);
			SQLGetData(hstmt, i, SQL_C_CHAR, col_name, GF_DB_STR_LEN, NULL);
			sprintf(format, "%%-%ds", (int)dlen + 1);
			printf(format, col_name, (int)dlen);
		}
		count++;
		printf("\n");
	}
	for(i = 1; i <= ncol; i++){
		SQLColAttribute(hstmt, i, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &dlen);
		printf("Col%d:%d ", i, (int)dlen);
	}
	printf("\n%d Rows\n", count);
	gf_db_free_stmt(hstmt);
	
	return 0;
}


