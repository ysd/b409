#include"global.h"
#include"utility.h"
int get_time_str(char *p,time_t t)
{
    int count = 0;
    long i = t;
    int j,k;
    while(i)
    {
        count++;
        i = i/10;
    }
    i = t;
    for(j = count-1;j>=0;j--)
    {
        k = i%10;
        p[j] = k + 48;
        i = i/10;
    }
    return 0;
}

int get_port_str(char *p,int port)
{
    int count = 0;
    int i = port;
    int j,k;
    while(i)
    {
        count++;
        i = i/10;
    }
    i = port;
    for(j = count-1;j>=0;j--)
    {
        k = i%10;
        p[j] = k + 48;
        i = i/10;
    }
    return 0;
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

int pthread_block_sig(int signo)
{
    int rt = 0;
    sigset_t st;
    sigemptyset(&st);
    sigaddset(&st,signo);
    rt = pthread_sigmask(SIG_BLOCK,&st,NULL);
    if(rt != 0)
    {
        printf("Block signal -- %d fail!\n",signo);
    }
    return rt;
}
int pthread_unblock_sig(int signo)
{
    int rt = 0;
    sigset_t st;
    sigemptyset(&st);
    sigaddset(&st,signo);
    rt = pthread_sigmask(SIG_UNBLOCK,&st,NULL);
    if(rt != 0)
    {
        printf("Unblock signal -- %d fail!\n",signo);
    }
    return rt;
}
