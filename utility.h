#ifndef _UTILITY_H
#define _UTILITY_H
extern int get_time_str(char *p,time_t t);
extern int get_port_str(char *p,int port);
extern int get_cache_path(char* path,char* cache_path);
extern int get_data_path(char* path,char* data_path);
extern int get_path_from_fd(int fd, char* path);
extern void rand_generator(int a[],int len,int limit);
extern void u64_to_str(u64 i,char *buf,u32 bufsiz);
extern void u32_to_str(u32 i,char *buf,u32 bufsiz);
extern void u8_to_str(char ch,char *buf,u32 bufsiz);
extern u64 atoi_u64(char *p);
extern u32 atoi_u32(char *p);
#endif
