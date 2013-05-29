#include"global.h"
#include"list_head.h"
#include"name_buf.h"
#include"nss.h"
#define OBJ_HASH_NR		4096
#define BKT_HASH_NR		2048
#define USER_HASH_NR	1024
static inline int __hash(char * name,int hash_nr)
{
	unsigned int seed = 131;
	unsigned int hash = 0;
	unsigned char * p = (unsigned char *)str;
	while(*p != '\0'){
		hash = hash * seed + (*p++);
	}
	return hash % hash_nr;
}
static inline int o_hash(char * obj_name)
{
	return __hash(obj_name,OBJ_HASH_NR);
}
static inline int b_hash(char * bucket_name)
{
	return __hash(bucket_name,BKT_HASH_NR);
}
static inline int u_hash(char * u_name)
{
	return __hash(u_name,USER_HASH_NR);
}
/*-------------------- split -----------------*/
static root_dir root;
static struct list_head * alloc_list_head_table(int nr)
{
	if(nr <= 0){
		return NULL;
	}
	struct list_head * lh = (struct list_head *)malloc(nr*LH_SZ);
	if(lh != NULL){
		while(nr > 0){
			list_head_init(&lh[--nr]);
		}
	}
	return lh;
}
static inline void free_list_head_table(struct list_head * lh)
{
	free(lh);
}
static void init_root(void)
{
	_uid(&root) = SU_UID;
	_gid(&root) = SU_GID;
	_acl(&root) |= (U_R | U_W | U_X);
	pthread_mutex_init(&root.mutex,NULL);
	list_head_init(&root.r_udirs);
	root.user_hashtable = alloc_list_head_table(USER_HASH_NR);
	return;
}
static user_dir_t * new_user_dir(name_zone_t user_name,u16 uid,u16 gid,u16 acl)
{
}
static bucket_t * new_bucket(name_zone_t bucket_name,u16 uid,u16 gid,u16 acl)
{
}
static object_t * new_object(name_zone_t object_name,u16 uid,u16 gid,u16 acl)
{
}
static int add_user(char * user_name,u16 init_acl)
{
	/* 1) check if user with same name has already exist 
	 * 2) if not,get a new user_dir_t for new user
	 * 3) get_name_zone for new user 
	 * 4) alloc hash_table for buckets of this user
	 * 5) add to the user list and user hash table */
}
static int add_bucket(user_dir_t * user,char * bucket_name,u16 init_acl)
{
}
static int add_object(bucket_t * bucket,char * object_name,u16 init_acl)
{
}
