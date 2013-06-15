#include"global.h"
#include"blk_idx.h"
/* fp is the finger print of the file name
 * a index file will be created
 * 0 is returned in success
 * 1 is returned in failure
 */
int init_index_file(char *fp)
{
	int fd;
	char index_file_name[MAX_PATH];
	get_index_file_name(fp,index_file_name);
	fd = creat(index_file_name,0660);
	if(fd == -1){
		perror("create index file");
		return 1;
	}
	close(fd);
	return 0;
}
int del_index_file(char * fp)
{
	char index_file_name[MAX_PATH];
	get_index_file_name(fp,index_file_name);
	if(unlink(index_file_name) != 0){
		perror("unlink index file");
		return 1;
	}
	return 0;
}
