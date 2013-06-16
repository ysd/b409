#include"global.h"
#include"bloom.h"
#include"md5.h"
#include"blk_idx.h"
#define _FILE_NAME	"/home/grant/Downloads/nc"
int main()
{
	struct stat st;
	u64 bytes_cached = 0;
	int fd,buflen,dedup_rt,ifd;
	index_entry_t idxe;
	u64 block_nr = 0;
	char buf[FIX_BLK_SZ],md5s[MD5_STRING_LEN],index_file_name[MAX_PATH];
	/* initialize index file */
	md5s_of_str(_FILE_NAME,strlen(_FILE_NAME),md5s);
	get_index_file_name(md5s,index_file_name);
	printf("index_file_name #%s\n",index_file_name);
	init_index_file(md5s);
	/* initialize index file ok */
	bloom_filter_t * bloom = bloom_create(BLOOM_FILTER_ZONE_SZ,3,simple_hash,RS_hash,JS_hash);
	show_bloom_info(bloom);
	fd = open(_FILE_NAME,O_RDONLY);
	if(fd == -1){
		perror("open");
		goto ret;
	}
	if(fstat(fd,&st) != 0){
		perror("fstat");
		goto close_fd_and_ret;
	}
	printf("original file size -- #%ld\n",st.st_size);
	bzero(buf,FIX_BLK_SZ);
	/* open index_file_name for write */
	ifd = open(index_file_name,O_RDWR);
	while((buflen = read(fd,buf,FIX_BLK_SZ)) > 0){
		md5s_of_str(buf,buflen,md5s);
		/* update the index file */
		bzero(&idxe,BLOCK_INDEX_ENTRY_SZ);
		strncpy(idxe.finger_print,md5s,MD5_STRING_LEN);
		clear_blk_d(&idxe);
		set_blk_p(&idxe);
		/* write to the index file */
		lseek(ifd,(block_nr++)*BLOCK_INDEX_ENTRY_SZ,SEEK_SET);
		if(write(ifd,&idxe,BLOCK_INDEX_ENTRY_SZ) != BLOCK_INDEX_ENTRY_SZ){
			perror("write block index extry");
			goto close_fd_and_ret;
		}
		dedup_rt = dedup(bloom,md5s,buf,buflen);
		if(dedup_rt == -1){
			goto close_fd_and_ret;
		}
		bytes_cached += dedup_rt;
		bzero(buf,FIX_BLK_SZ);
	}
	printf("--------- block_nr #%ld\n",block_nr);
	/* before close index file,read it to see if the index entry is put into file correctly */
	block_nr=0;
	bzero(&idxe,BLOCK_INDEX_ENTRY_SZ);
	lseek(ifd,0,SEEK_SET);
	while(read(ifd,&idxe,BLOCK_INDEX_ENTRY_SZ) == BLOCK_INDEX_ENTRY_SZ){
		prt_idxe(&idxe);
		bzero(&idxe,BLOCK_INDEX_ENTRY_SZ);
	}
	close(ifd);
	printf("bytes actually cached -- #%ld\n",bytes_cached);
	show_bloom_info(bloom);

	/* dedup a totally identical file */
	printf("------------ dedup a totally identical file ------------\n");
	printf("original file size -- #%ld\n",st.st_size);
	lseek(fd,0,SEEK_SET);
	bytes_cached = 0;
	bzero(buf,FIX_BLK_SZ);
	while((buflen = read(fd,buf,FIX_BLK_SZ)) > 0){
		md5s_of_str(buf,buflen,md5s);
		dedup_rt = dedup(bloom,md5s,buf,buflen);
		if(dedup_rt == -1){
			goto close_fd_and_ret;
		}
		bytes_cached += dedup_rt;
		bzero(buf,FIX_BLK_SZ);
	}
	printf("bytes actually cached -- #%ld\n",bytes_cached);
	show_bloom_info(bloom);
close_fd_and_ret:
	close(fd);
ret:
	bloom_destroy(bloom);
	return 0;
}
