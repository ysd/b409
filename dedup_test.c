#include"global.h"
#include"bloom.h"
#define _FILE_NAME	"/home/libxml2-2.7.8.tar.gz"
int main()
{
	struct stat st;
	u64 bytes_cached = 0;
	int fd,buflen,dedup_rt;
	char buf[FIX_BLK_SZ];
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
	while((buflen = read(fd,buf,FIX_BLK_SZ)) > 0){
		dedup_rt = dedup(bloom,buf,buflen);
		if(dedup_rt == -1){
			goto close_fd_and_ret;
		}
		bytes_cached += dedup_rt;
		bzero(buf,FIX_BLK_SZ);
	}
	printf("bytes actually cached -- #%ld\n",bytes_cached);
	show_bloom_info(bloom);

	/* dedup a totally identical file */
	printf("------------ dedup a totally identical file ------------\n");
	printf("original file size -- #%ld\n",st.st_size);
	lseek(fd,0,SEEK_SET);
	bytes_cached = 0;
	bzero(buf,FIX_BLK_SZ);
	while((buflen = read(fd,buf,FIX_BLK_SZ)) > 0){
		dedup_rt = dedup(bloom,buf,buflen);
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
