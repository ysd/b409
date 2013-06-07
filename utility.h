#ifndef _UTILITY_H
#define _UTILITY_H
_PROTOTYPE(int get_time_str,(char *p,time_t t));
_PROTOTYPE(int get_port_str,(char *p,int port));
_PROTOTYPE(int get_cache_path,(char* path,char* cache_path));
_PROTOTYPE(int get_data_path,(char* path,char* data_path));
_PROTOTYPE(int get_path_from_fd,(int fd, char* path));
_PROTOTYPE(void rand_generator,(int a[],int len,int limit));
_PROTOTYPE(void u64_to_str,(u64 i,char *buf,u32 bufsiz));
_PROTOTYPE(void u32_to_str,(u32 i,char *buf,u32 bufsiz));
_PROTOTYPE(void u8_to_str,(char ch,char *buf,u32 bufsiz));
_PROTOTYPE(u64 atoi_u64,(char *p));
_PROTOTYPE(u32 atoi_u32,(char *p));
#endif
