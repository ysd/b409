#include"global.h"
#include"utility.h"
#include"blk_idx.h"
static int do_data_center_read(char * file_name,u8 *buf,u32 count,u64 offset)
{
	int fd;
	char dst_path[MAX_PATH];
	get_data_path(file_name,dst_path);
	if((fd = open(dst_path,O_RDONLY)) == -1){
		perror("open data center file for read");
		return 1;
	}
	lseek(fd,offset,SEEK_SET);
	if(read(fd,buf,count) != count){
		perror("read from data center");
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}
static int do_data_center_write(char * file_name,u8 *buf,u32 count,u64 offset)
{
	int fd;
	char dst_path[MAX_PATH];
	get_data_path(file_name,dst_path);
	if((fd = open(dst_path,O_WRONLY)) == -1){
		perror("open data center file for write");
		return 1;
	}
	lseek(fd,offset,SEEK_SET);
	if(write(fd,buf,count) != count){
		perror("write to data center");
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}
static int do_rm_data_center_file(char * file_name)
{
	char dst_path[MAX_PATH];
	get_data_path(file_name,dst_path);
	return unlink(dst_path);
}
static int do_cache_read(char * file_name,u8 *buf,u32 count,u64 offset)
{
	int fd;
	u64 block_nr;
	u32 in_blk_start,in_blk_count,bytes_read = 0;
	index_entry_t idx;
	char index_file[MAX_PATH];
	get_index_file_name(file_name,index_file);

	block_nr = offset/FIX_BLK_SZ;
	in_blk_start = offset%FIX_BLK_SZ;
	in_blk_count = zmin(FIX_BLK_SZ-in_blk_start,count);
	/*
	 * read all the blk index entrys needed here into memory first 
	 * read blk index entrys
	 */
	 //	get_idxe();
	while(bytes_read != count){
		if((fd = open(idx.finger_print,O_RDONLY)) < 0){
			perror("open block file read");
			return 1;
		}
		lseek(fd,in_blk_start,SEEK_SET);
		if(read(fd,buf+bytes_read,in_blk_count) != in_blk_count){
			perror("read block file");
			close(fd);
			return 1;
		}
		close(fd);
		bytes_read += in_blk_count;
		//get_idxe();
		in_blk_start = 0;
		in_blk_count = zmin(count-bytes_read,FIX_BLK_SZ);
	}
	return 0;
}
static int do_cache_write(char * file_name,u8 *buf,u32 count,u64 offset)
{
	int fd;
	u64 block_nr;
	u32 in_blk_start,in_blk_count,bytes_written = 0;
	index_entry_t idx;
	char index_file[MAX_PATH];
	get_index_file_name(file_name,index_file);

	block_nr = offset/FIX_BLK_SZ;
	in_blk_start = offset%FIX_BLK_SZ;
	in_blk_count = zmin(FIX_BLK_SZ-in_blk_start,count);
	/*
	 * read all the blk index entrys needed here into memory first 
	 * read blk index entrys
	 */
	 //	get_idxe();
	while(bytes_written != count){
		if((fd = open(idx.finger_print,O_WRONLY)) < 0){
			perror("open block file for write");
			return 1;
		}
		lseek(fd,in_blk_start,SEEK_SET);
		if(write(fd,buf+bytes_written,in_blk_count) != in_blk_count){
			perror("write block file");
			close(fd);
			return 1;
		}
		close(fd);
		bytes_written += in_blk_count;
		//get_idxe();
		in_blk_start = 0;
		in_blk_count = zmin(count-bytes_written,FIX_BLK_SZ);
	}
	return 0;
}
static int do_rm_cache_file(char * file_name)
{
}
/* int _creat(char * file_name) 
 * @file_name : specify the file to be created,
 * which is identified by the md5 of the full path in the namespace
 *
 * Tips : Object A is only created in the first put_object(A) request,
 * if object A already exists(when it is already in namespace),
 * it will not be created again.
 * The default operation is that file is only created in data center.
 * */
int _creat(char * file_name)
{
	char dst_path[MAX_PATH];
	get_data_path(file_name,dst_path);
	if(creat(dst_path,0660) == -1){
		perror("create data center file");
		return 1;
	}
	close(fd);
	return 0;
}
int _read(char * file_name,u8 * buf,u32 count,u64 offset)
{
	meta_data_t md;
	if(md_get(file_name,&md) != 0){
		return 1;
	}
	if(is_in_cache(&md)){
		if(do_cache_read(file_name,buf,count,offset) != 0){
			fprintf(stderr,"do_cache_read fail!\n");
			return 1;
		}
	}else{
		if(do_data_center_read(file_name,buf,count,offset) != 0){
			fprintf(stderr,"do_data_center_read fail!\n");
			return 1;
		}
	}
	return 0;
}
int _write(char * file_name,u8 * buf,u32 count,u64 offset)
{
	meta_data_t md;
	if(md_get(file_name,&md) != 0){
		return 1;
	}
	if(is_in_cache(&md)){
		if(do_cache_write(file_name,buf,count,offset) != 0){
			fprintf(stderr,"do_cache_write fail!\n");
			return 1;
		}
	}else{
		if(do_data_center_write(file_name,buf,count,offset) != 0){
			fprintf(stderr,"do_data_center_write fail!\n");
			return 1;
		}
	}
	return 0;
}
int _rm(char * file_name)
{
	meta_data_t md;
	if(md_get(file_name,&md) != 0){
		return 1;
	}
	if(rm_data_center_file(file_name) != 0){
		fprintf(stderr,"rm_data_center_file fail!\n");
		return 1;
	}
	if(is_in_cache(&md)){
		if(rm_cache_file(file_name) != 0){
			fprintf(stderr,"rm_cache_file fail!\n");
			return 1;
		}
	}
	return 0;
}
