#include"global.h"
#include"posix_api.h"
void main(int argc,char **argv)
{
    int n = 0;
    int fd;
    char *path;
    char buf[BUFSIZ];
    if(argc != 2){
        printf("invalid argument!\n");
        exit(1);
    }
    path = argv[1];
    fd = _OpenFile(path,O_RDONLY);
    printf("readfile fd -- %d\n",fd);
    if(fd != -1){
        bzero(buf,BUFSIZ);
        n = _ReadFile(fd,buf,BUFSIZ,0);
        printf("read -- %s\n",buf);
        _CloseFile(fd);
    }
    exit(0);
}
