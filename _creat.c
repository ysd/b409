#include"global.h"
#include"posix_api.h"
void main(int argc,char **argv)
{
    int fd;
    char *path;
    if(argc != 2){
        printf("invalid argument!\n");
        exit(1);
    }
    path = argv[1];
    fd = _CreateFile(path,0770,O_RDONLY);
    printf("Creat file -- %s  fd -- %d\n",path,fd);
    _CloseFile(fd);
    exit(0);
}
