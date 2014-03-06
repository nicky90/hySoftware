/*
 * gferror.h - error code and log head file
 *
 * Writen by CX3201 2013-05-29
 */

#ifndef __GFERROR_H__
#define __GFERROR_H__

#include <stdio.h>
#include <errno.h>

/* Success is the fire error code, remember to modify */
#define GFSUCCESS       0 /* Success */
#define GFNULLPARAM     1 /* NULL parameter */
#define GFLONGPARAM     2 /* Parameter too long */
#define GFINVAPARAM     3 /* Invalid parameter */
#define GFSTATEERR      4 /* Not in right state */
#define GFALLOCERR      5 /* Allocate error */
#define GFSETERR        6 /* Set attribute error */
#define GFCONNERR       7 /* Connect failed */
#define GFEXECERR       8 /* Execute error */
#define GFAGAIN         9 /* Try again */
#define GFRESOK        10 /* Respond OK */
#define GFRESDENYP     11 /* Request access deny */
#define GFRESETYPE     12 /* Request type error */
#define GFRESNREQ      13 /* Request parameter error */
#define GFRESNEXIST    14 /* Request resource not exist */
#define GFCOMPERR      15 /* Compress failed */
#define GFUNCOMPERR    16 /* Uncompress failed */
#define GFRDNERR       17 /* Read nbytes failed*/
#define GFWRNERR       18 /* Write nbytes failed*/
#define GFRDLINERR     19 /* Read a line failed*/
#define GFWRLINERR     20 /* Write a line failed*/
#define GFSETRDTERR    21 /* Set read timeout error*/
#define GFSETWRTERR    22 /* Set Write timeout error*/
#define GFSOCKSETREUSEERR 	23 /*Set socket reuse error*/
#define GFSOCKBINDERR  		24 /*Socket bind error*/
#define GFSOCKLISTENERR		25 /*Socket listen error*/
#define GFSOCKCREATERR		26 /*Socket creat error*/
#define GFTHREADCREATERR	27 /*Thread creat error*/
#define GFEXIST        28 /* Resource exists */
#define GFNEXIST       29 /* Resource not exists */
#define GFDATAERR      30 /* Data error */
#define GFEOF          31 /* Get EOF */
#define GFCONFERR      32 /* Config error */
#define GFCREATEERR    33 /* Create file error */
#define GFOPENERR      34 /* Open file error */
#define GFDELERR       35 /* Delete file error */
#define GFLOCKERR      36 /* Lock source error */
#define GFUNLOCKERR    37 /* Unlock source error */
#define GFMAKEERR      38 /* Make resource error */
#define GFCREATETHERR  39 /* Create thread error */
#define GFFORKERR      40 /* Fork error */
#define GFABORT        41 /* Proccess abort */
#define GFRESATNOFIND  42 /* Re get sat name error*/
#define GFREOPENFIERR  43 /* Re open file error*/
#define GFRESEEKFIERR  44 /* Re seek file error*/
#define GFREREADFIERR  45 /* Re read file error*/
#define GFRERDSIZEERR  46 /* Re read size error*/
#define GFRESEGNOFULL  47 /* Re segment error*/
#define GFRELEVELERR   48 /* Re no such level*/
#define GFRESTATFIERR  49 /* Re no such path*/
#define GFRESTATDIERR  50 /* Re path is not dir*/
#define GFREOPENDIERR  51 /* Re open dir error*/
/* Unknown is the last error code, remember to modify */
#define GFUNKOWEN      52 /* Unknown error */
#define GF_RET_CODE(x)  (((x)>=GFSUCCESS)&&((x)<=GFUNKOWEN))

#define GF_ERROR_STR {\
	"Success.",\
	"NULL parameter.",\
	"Parameter too long.",\
	"Invalid parameter.",\
	"Not in right state.",\
	"Allocate error.",\
	"Set attribute error.",\
	"Connect failed.",\
	"Execute error.",\
	"Try again.",\
	"Respond OK.",\
	"Request access deny.",\
	"Request type error.",\
	"Request parameter error.",\
	"Request resource not exist.",\
	"Compress failed", \
	"Uncompress failed", \
	"Read nbytes failed", \
	"Write nbytes failed", \
	"Read a line failed", \
	"Write a line failed", \
	"Set read timeout error", \
	"Set Write timeout error", \
	"Set socket reuse error", \
	"Socket bind error", \
	"Socket listen error", \
	"Socket creat error", \
	"Thread creat error", \
	"Resource exists",\
	"Resource not exists",\
	"Data error",\
	"Get EOF",\
	"Config error",\
	"Create file error",\
	"Open file error",\
	"Delete file error",\
	"Lock source error",\
	"Unlock source error",\
	"Make resource error",\
	"Create thread error",\
	"Fork error",\
	"Proccess abort",\
	"Re get sat name error",\
	"Re open file error",\
	"Re seek file error",\
	"Re read file error",\
	"Re read size error",\
	"Re segment error",\
	"Re no such level",\
	"Re no such path",\
	"Re path is not dir",\
	"Re open dir error",\
	"Unknown error."\
}

#define GF_ERROR_PR(x)    {int t=x;printf("%s:%s:%d:%s\n",__FILE__,__FUNCTION__,__LINE__,gf_strerr(x));errno=t;return -1;}
#define GF_ERROR_NP(x)    {errno=x;return -1;}
#define GF_PRINTERR(x)    printf("%s:%s:%d:%s\n",__FILE__,__FUNCTION__,__LINE__,gf_strerr(x))

#ifdef  GF_INCLUDE_DATABASE
#include "gftmdb.h"
#define GF_ERROR_DB(x)  {int t=x;gf_tm_debug(x,__FILE__,__FUNCTION__,__LINE__,gf_strerr(x));errno=t;return -1;}
#define GF_ABORT_DB(x)  {int t=x;gf_tm_debug(x,__FILE__,__FUNCTION__,__LINE__,gf_strerr(x));errno=t;gf_tm_refresh_state("ABORT");exit(-1);}
#define GF_ERROR_LOG(x) {int t=x;gf_tm_debug(x,__FILE__,__FUNCTION__,__LINE__,gf_strerr(x));errno=t;}
#define GF_WARNING(x)   gf_tm_debug(0,__FILE__,__FUNCTION__,__LINE__,x)
#define GF_ERROR(x)     GF_ERROR_DB(x)
#define GF_ABORT(x)     GF_ABORT_DB(x)
#else
#define GF_ERROR(x)     GF_ERROR_PR(x)
#endif /* GF_INCLUDE_DATABASE */

/* Get error string */
extern char* gf_strerr(int err);

#endif /* __GFERROR_H__ */

