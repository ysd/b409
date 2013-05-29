#ifndef _UTILITY_H
#define _UTILITY_H
_PROTOTYPE(int get_time_str,(char *p,time_t t));
_PROTOTYPE(int get_port_str,(char *p,int port));
_PROTOTYPE(int get_cache_path,(char* path,char* cache_path));
_PROTOTYPE(int get_data_path,(char* path,char* data_path));
_PROTOTYPE(int get_path_from_fd,(int fd, char* path));
_PROTOTYPE(int pthread_block_sig,(int signo));
_PROTOTYPE(int pthread_unblock_sig,(int signo));
_PROTOTYPE(void rand_generator,(int a[],int len));
#endif
