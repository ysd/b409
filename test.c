#include"global.h"
#include"bloom.h"
#include"dedup.h"
#define _FILE_NAME	""
#define BUFLEN	4096
int main()
{
	int fd,buflen;
	char buf[BUFLEN];
	bloom_filter_t * bloom = bloom_create(2048,3,simple_hash,RS_hash,JS_hash);
	fd = open(_FILE_NAME,O_RDONLY);
	bzero(buf,BUFLEN);
	while((buflen = read(fd,buf,BUFLEN)) > 0){
		dedup(bloom,buf,buflen);
		bzero(buf,BUFLEN);
	}
	bloom_destroy(bloom);
	return 0;
}
