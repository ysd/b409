#ifndef _NSS_API_H
#define _NSS_API_H
#include"nss.h"
extern void init_name_space(void);
extern int put_object(char * object_name,char * bucket_name,char * user_name);
extern int get_object(char * object_name,char * bucket_name,char * user_name);
extern int delete_object(char * object_name,char * bucket_name,char * user_name);

extern int put_bucket(char * bucket_name,char * user_name);
extern int get_bucket(char * bucket_name,char * user_name);
extern int delete_bucket(char * bucket_name,char * user_name);

extern int put_user(char * user_name);
extern int get_user(char * user_name);
extern int delete_user(char * user_name);

extern int get_user_dir_by_name(char * name,void ** ptr,const u8 op_style);
extern int get_bucket_by_name(char * name,user_dir_t * user,void ** ptr,const u8 op_style);
extern int get_object_by_name(char * name,bucket_t * bucket,void ** ptr,const u8 op_style);
/* for debug */
extern void prt_olist(bucket_t * bucket);
extern void prt_blist(user_dir_t * user);
extern void prt_ulist(void);
extern void prt_uhash(void);
extern void prt_bhash(void);
extern void prt_ohash(void);
#endif
