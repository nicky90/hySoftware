/*
 * gferror.h - database utils
 *
 * Writen by CX3201 2013-05-31
 */

#ifndef __GFDBUTILS_H__
#define __GFDBUTILS_H__

#ifdef GF_INCLUDE_DATABASE

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#define SQL_OK(r)          ((SQL_SUCCESS == r) || (SQL_SUCCESS_WITH_INFO == r))
#define GF_DB_STR_LEN      1024
#define GF_DB_NAME_LEN     64
#define GF_DB_CONN_TIMEOUT 5
#define GF_DB_RETRY        1

/* Database description struct */
typedef struct __gf_db_t{
	int     isinit;
	int     isconn;
	SQLHENV henv;
	SQLHDBC hdbc;
	char    dsn[GF_DB_NAME_LEN];
	char    user[GF_DB_NAME_LEN];
	char    psw[GF_DB_NAME_LEN];
} gf_db_t;

/* Gerneral use functions */
int gf_db_init(char *dsn, char *user, char *psw);
int gf_db_conn(void);
int gf_db_disconn(void);
int gf_db_is_conn(void);
int gf_db_get_stmt(SQLHSTMT *phstmt);
int gf_db_free_stmt(SQLHSTMT hstmt);
int gf_db_query_with_reconn(SQLHSTMT *phstmt, char * sql);
int gf_db_ping(void);

/* Special use functions */
int gf_db_query_test(char *sql);

#else /* GF_INCLUDE_DATABASE */
#error Need GF_INCLUDE_DATABASE macro
#endif /* GF_INCLUDE_DATABASE */

#endif /* __GFDBUTILS_H__ */
