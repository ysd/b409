#include"global.h"
#include"bloom.h"
#include"md5.h"
/*  1 is returned on success
 * -1 is returned on failure */
int dedup(bloom_filter_t *bloom,u8 * buf,u32 buflen)
{
	int i,fd;
	char blk_cache_path[MAX_PATH];
	u8 md5c[MD5_CHECKSUM_SZ],md5s[MD5_STRING_LEN];
	md5(buf,buflen,md5c);
	bzero(md5s,MD5_STRING_LEN);
	md5_2_str(md5c,md5s);
	i = bloom_op(bloom,md5s,BLOOM_INC);
	if(i == -1){
		fprintf(stderr,"BLOOM_INC fail!\n");
		return -1;
	}
	if(i == 1){
		/* just return */
		return 1;
	}
	/* store the blk file in CACHE_PATH */
	bzero(blk_cache_path,MAX_PATH);
	snprintf(blk_cache_path,MAX_PATH,"%s%s",CACHE_PATH,md5s);
	fd = open(blk_cache_path,O_CREAT | O_RDWR,0660);
	if(fd == -1){
		perror("open");
		return -1;
	}
	if(write(fd,buf,buflen) == -1){
		perror("write");
		close(fd);
		return -1;
	}
	close(fd);
	return 1;
}
