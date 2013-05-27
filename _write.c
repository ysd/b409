#include"global.h"
#include"posix_api.h"
void main(int argc,char **argv)
{
    int n = 0;
    int fd;
    char *path;
    char *buf;
    if(argc != 3){
        printf("invalid argument!\n");
        exit(1);
    }
    path = argv[1];
    buf = argv[2];
    fd = _OpenFile(path,O_WRONLY);
    printf("writefile fd -- %d\n",fd);
    if(fd != -1){
        n = _WriteFile(fd,buf,strlen(buf),0);
        printf("%d byte(s) write\n",n);
        _CloseFile(fd);
    }
    exit(0);
}
