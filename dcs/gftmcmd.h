/*
 * gftmcmd.h - define routines used by UI 
 */

#ifndef GF_TM_CMD_H__
#define GF_TM_CMD_H__

#include <stdint.h>

typedef struct __ui_msg_t {
	char req_name[20];
	char req_args[5][50];
} ui_msg_t;


/* restart or shutdown  BJ and/or MDJ servers, \
 *   as described by 'station'
 *
 * when 'station' is bj, restart bj only;
 * when 'station' is md, restart md only
 * or when station is both, restart both
 */
int gf_restart(const char *station);
int gf_shutdown(const char *station);


#endif
