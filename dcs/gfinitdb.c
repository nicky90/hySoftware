
#include "gftm.h"
#include "gferror.h"
#include "gftmdb.h"

/*---------------------- */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int gf_tm_init(char *dsn, char *usr, char *psw)
{
	/* Database init */
	if(gf_db_init(dsn, usr, psw)) GF_ERROR_PR(errno);
	if(gf_db_conn()) GF_ERROR_PR(errno);
    gf_tm_init_tables();
	
	return 0;
}

/*************Main*************/
int main(int argc, char *argv[])
{
    
	if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) GF_ERROR_PR(errno);

    printf("DATABASE INITIATED!!\n");
	exit(0);
}

