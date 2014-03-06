/*
 * gfsignal.c - gf signal processing routines
 */

#include "gfsignal.h"

#include <stdlib.h>
#include <signal.h>

static void gf_sig_term(int sig)
{
	exit(0);
}

//static void gf_sig_restart(int sig);

int gf_init_signal(void)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGXCPU, SIG_DFL);
	signal(SIGXFSZ, SIG_DFL);
	signal(SIGINT, gf_sig_term);
	signal(SIGTERM, gf_sig_term);

	return 0;
} 
