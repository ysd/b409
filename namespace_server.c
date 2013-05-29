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
	bzero(&root,ROOT_DIR_SZ);
	root.root = ROOT_DIR;
	_uid(&root) = SU_UID;
	_gid(&root) = SU_GID;
	_acl(&root) |= (U_R | U_W | U_X);
	pthread_mutex_init(&root.mutex,NULL);
	list_head_init(&root.r_udirs);
	root.user_hashtable = alloc_list_head_table(USER_HASH_NR);
	return;
}
static u_dir new_udir(char * dir_name)
