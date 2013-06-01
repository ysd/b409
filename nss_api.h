#ifndef _NSS_API_H
#define _NSS_API_H
#include"nss.h"
extern void init_name_space(void);
extern int add_user(char * user_name);
extern int add_bucket(char * bucket_name,user_dir_t * user);
extern int add_object(char * object_name,bucket_t * bucket);
extern int del_object(object_t * object);
extern int del_bucket(bucket_t * bucket);
extern int del_user(user_dir_t * user);
extern void get_absolute_path_of_object(object_t * object,char name_buf[]);
extern void list_bucket(user_dir_t * user);
extern void list_object(bucket_t * bucket);
extern void prt_olist(bucket_t * bucket);
extern void prt_blist(user_dir_t * user);
extern void prt_ulist(void);
#endif
