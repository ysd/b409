#include"global.h"
#include"utility.h"
void rand_generator(int a[],int len)
{
	int i;
	for(i = 0;i < len;i++){
		srand(time(NULL) + i);
		a[i] = rand()%1000;
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
