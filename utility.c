#include"global.h"
#include"utility.h"
#include"nss.h"
void rand_generator(int a[],int len,int limit)
{
	int i;
	for(i = 0;i < len;i++){
		srand(time(NULL) + i);
		a[i] = rand()%limit;
	}
	return;
}
int get_cache_path(char* path,char* cache_path)
{
    char *p;
	bzero(cache_path,MAX_PATH);
    p = cache_path + strlen(CACHE_PATH);
    strcpy(cache_path,CACHE_PATH);
    strcpy(p,path);
    return 1;
}
int get_data_path(char* path,char* data_path)
{
    char *p;
	bzero(data_path,MAX_PATH);
    p = data_path + strlen(DATA_PATH);
    strcpy(data_path,DATA_PATH);
    strcpy(p,path);
    return 1;
}
int get_path_from_fd(int fd, char* path)
{
    char buf[1024];
	pid_t  pid;
    bzero(buf, 1024);
    pid = getpid();
	bzero(path,MAX_PATH);
    snprintf(buf, 1024, "/proc/%i/fd/%i", pid, fd);
    return readlink(buf, path, MAX_PATH);
}
u32 atoi_u32(char *p)
{
	u32 i = 0;
	u32 j;
	char * ch = p;
	while(*ch != 00){
		if(*ch < '0' || *ch > '9'){
			fprintf(stderr,"Invalid character found in u32 string!\n");
			return -1;
		}
		j = (u32)(*ch - '0');
		i = 10*i + j;
		ch++;
	}
	return i;
}
u64 atoi_u64(char *p)
{
	u64 i = 0;
	u64 j;
	char * ch = p;
	while(*ch != 00){
		if(*ch < '0' || *ch > '9'){
			fprintf(stderr,"Invalid character found in u64 string!\n");
			return -1;
		}
		j = (u64)(*ch - '0');
		i = 10*i + j;
		ch++;
	}
	return i;
}
void u8_to_str(char ch,char *buf,u32 bufsiz)
{
	bzero(buf,bufsiz);
	snprintf(buf,bufsiz,"%c",ch);
}
void u32_to_str(u32 i,char *buf,u32 bufsiz)
{
	bzero(buf,bufsiz);
	snprintf(buf,bufsiz,"%u",i);
}
void u64_to_str(u64 i,char *buf,u32 bufsiz)
{
	bzero(buf,bufsiz);
	//snprintf(buf,bufsiz,"%llu",i);
	snprintf(buf,bufsiz,"%"PRIu64"",i);
}
