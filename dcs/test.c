/*
 * testdbutils.c - database utils test
 *
 * Writen by CX3201 2013-05-31
 */

#include "../gfdbutils.h"
#include "gftmdb.h"
#include "gftm.h"
#include "../gferror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int ret, i, j, k;
	gf_tm_task_t task;
	gf_tm_chn_t  chn;
	gf_tm_file_t file;
	char sql[1024], cmd[256];

	if(argc > 2){
		printf("Usage: test [recreate|filltable|num]\n");
		exit(-1);
	}
	if(argc == 2){
		if(strcmp(argv[1], "recreate") == 0){
			if(gf_db_init("GFGroupDB","GFGroup","13910821754!!")) GF_ERROR_PR(errno);
			if(gf_db_conn()) GF_ERROR_PR(errno);
			if(gf_tm_init_tables()){
				gf_tm_destroy();
				GF_ERROR_PR(errno);
			}
			if(gf_db_disconn()) GF_ERROR_PR(errno);
		}else if(strcmp(argv[1], "filltable") == 0){
			if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) exit(-1);
			for(i = 1; i <= 100; i++){
				sprintf(task.orb, "%06d", i);
				gf_tm_add_task("GF01", task.orb, 2);
				for(j = 1; j <= 2; j++){
					sprintf(chn.chn, "%02d", j);
					gf_tm_add_chn("GF01", task.orb, chn.chn);
					for(k = 1; k <= 100; k++){
						sprintf(file.filename, "file_%03d.dat", k);
						gf_tm_add_file("GF01", task.orb, chn.chn, file.filename, k, (long)60<<30,4<<20);
					}
				}
			}
			gf_tm_destroy();
		}else if(strcmp(argv[1], "restore") == 0){
			if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) exit(-1);
			for(i = 1; i <= 100; i++){
				sprintf(file.orb, "%06d", i);
				for(j = 1; j <= 2; j++){
					sprintf(file.chn, "%02d", j);
					for(k = 1; k <= 100; k++){
						sprintf(file.filename, "file_%03d.dat", k);
						gf_tm_restore_file_state("GF01",file.orb,file.chn,file.filename);
					}
				}
			}
			gf_tm_destroy();
		}else{
			i = atoi(argv[1]);
			if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) exit(-1);
			while(i--){
				if(gf_tm_get_top_file(&file)){
					GF_PRINTERR(errno);
					gf_tm_destroy();
					printf("%d tests left\n", i);
					exit(-1);
				}else gf_tm_change_file_state(file.sat,file.orb,file.chn,file.filename,"HANDLE",1);
			}
			gf_tm_destroy();
		}
		exit(0);
	}
	if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) exit(-1);
	while(1){
		printf("cmd>");
		scanf("%s", cmd);
		if(strcmp(cmd,"quit") == 0) break;
		if(strcmp(cmd,"et") == 0){
			scanf("%s%s", task.sat, task.orb);
			ret = gf_tm_exist_task(task.sat, task.orb, NULL);
			if(ret == 1) printf("Exist %s %s\n", task.sat, task.orb);
			else if(ret == 0) printf("Not Exist %s %s\n", task.sat, task.orb);
			else GF_PRINTERR(errno);
			continue;
		}
		if(strcmp(cmd,"ec") == 0){
			scanf("%s%s%s", chn.sat, chn.orb, chn.chn);
			ret = gf_tm_exist_chn(chn.sat, chn.orb, chn.chn, NULL);
			if(ret == 1) printf("Exist %s %s %s\n", chn.sat, chn.orb, chn.chn);
			else if(ret == 0) printf("Not Exist %s %s %s\n", chn.sat, chn.orb, chn.chn);
			else GF_PRINTERR(errno);
			continue;
		}
		if(strcmp(cmd,"ef") == 0){
			scanf("%s%s%s%s", file.sat, file.orb, file.chn, file.filename);
			ret = gf_tm_exist_file(file.sat, file.orb, file.chn, file.filename, NULL);
			if(ret == 1) printf("Exist %s %s %s %s\n", file.sat, file.orb, file.chn, file.filename);
			else if(ret == 0) printf("Not Exist %s %s %s %s\n", file.sat, file.orb, file.chn, file.filename);
			else GF_PRINTERR(errno);
			continue;
		}
		if(strcmp(cmd,"at") == 0){
			scanf("%s%s", task.sat, task.orb);
			if(gf_tm_add_task(task.sat, task.orb, 2)){
				if(GFEXIST == errno) printf("Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"ac") == 0){
			scanf("%s%s%s", chn.sat, chn.orb, chn.chn);
			if(gf_tm_add_chn(chn.sat, chn.orb, chn.chn)){
				if(GFEXIST == errno) printf("Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"af") == 0){
			scanf("%s%s%s%s", file.sat, file.orb, file.chn, file.filename);
			if(gf_tm_add_file(file.sat, file.orb, file.chn, file.filename, 1, 123456789012345,12345678)){
				if(GFEXIST == errno) printf("Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"cts") == 0){
			scanf("%s%s%s", task.sat, task.orb, task.state);
			if(gf_tm_change_task_state(task.sat, task.orb, task.state, 1)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"ccs") == 0){
			scanf("%s%s%s%s", chn.sat, chn.orb, chn.chn, chn.state);
			if(gf_tm_change_chn_state(chn.sat, chn.orb, chn.chn, chn.state, 1)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"cfs") == 0){
			scanf("%s%s%s%s%s", file.sat, file.orb, file.chn, file.filename, file.state);
			if(gf_tm_change_file_state(file.sat, file.orb, file.chn, file.filename, file.state, 0)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"rt") == 0){
			scanf("%s%s", task.sat, task.orb);
			if(gf_tm_add_task_retran(task.sat, task.orb)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"rc") == 0){
			scanf("%s%s%s", chn.sat, chn.orb, chn.chn);
			if(gf_tm_add_chn_retran(chn.sat, chn.orb, chn.chn)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"rf") == 0){
			scanf("%s%s%s%s", file.sat, file.orb, file.chn, file.filename);
			if(gf_tm_add_file_retran(file.sat, file.orb, file.chn, file.filename)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"afrs") == 0){
			scanf("%s%s%s%s%ld", file.sat, file.orb, file.chn, file.filename, &(file.recv_size));
			if(gf_tm_add_file_recv_size(file.sat, file.orb, file.chn, file.filename, file.recv_size)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"sfrs") == 0){
			scanf("%s%s%s%s%ld", file.sat, file.orb, file.chn, file.filename, &(file.recv_size));
			if(gf_tm_set_file_recv_size(file.sat, file.orb, file.chn, file.filename, file.recv_size)){
				if(GFNEXIST == errno) printf("Not Exist\n");
				else GF_PRINTERR(errno);
			}
			continue;
		}
		if(strcmp(cmd,"gtf") == 0){
			if(gf_tm_get_top_file(&file)){
				GF_PRINTERR(errno);
			}else gf_dbg_print_file(&file);
			continue;
		}
	}
	gf_tm_destroy();

	return 0;
}
