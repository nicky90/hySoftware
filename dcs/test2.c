#include "gftmconfig.h"
#include "../gferror.h"

int main()
{
	int count = 0;
	char ch;
	int64_t index;
	gf_tm_file_t file;

	if(gf_tm_read_config("tm.cfg")) GF_ERROR_PR(errno);
	if(gf_tm_read_sat_config("tmsat.cfg")) GF_ERROR_PR(errno);
	gf_tm_print_config();
	gf_tm_print_sat_config();
	if(gf_tm_init("GFGroupDB","GFGroup","13910821754!!")) GF_ERROR_PR(errno);
	gf_tm_get_top_file(&file);
	while(1){
		count++;
		printf("cmd>");
		ch = getchar();
		if('q' == ch) break;
		//gf_tm_delete_check_file(&file);
		gf_tm_get_top_file(&file);
		printf("%05d | %s\n", count, file.filename);
		//gf_tm_create_check_file(&file);
		//gf_tm_change_file_state(file.sat,file.orb,file.chn,file.filename,"HANDLE",1);
		while((index = gf_tm_get_top_block(&file)) > 0) printf("GetBlock %ld\n", index);
		if(index) GF_ERROR_PR(errno);
		if(gf_tm_recover_check_file(&file)) GF_ERROR_PR(errno);
		printf("pausing...");
		getchar();
	}
	gf_tm_destroy();
	return 0;
}
