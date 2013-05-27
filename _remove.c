#include"global.h"
#include"posix_api.h"
void main(int argc,char **argv)
{
    char *path;
    if(argc != 2){
        printf("invalid argument!\n");
        exit(1);
    }
    path = argv[1];
    if(_Remove(path) == 0){
        printf("remove success!\n");
    }
    exit(0);
}
