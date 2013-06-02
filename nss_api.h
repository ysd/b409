#ifndef _NSS_API_H
#define _NSS_API_H
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

/* for debug */
extern void prt_olist(bucket_t * bucket);
extern void prt_blist(user_dir_t * user);
extern void prt_ulist(void);
extern void prt_uhash(void);
extern void prt_bhash(void);
extern void prt_ohash(void);
#endif
