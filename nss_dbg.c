#include"global.h"
#include"list_head.h"
#include"name_buf.h"
#include"nss.h"
/* for debug */
void prt_ulist(void)
{
	struct list_head * l;
	user_dir_t * u;
	for_each_user(l){
		u = container_of(l,user_dir_t,u_list);
		printf("%s - ",*(u->user_name));
	}
	printf("\n");
}
void prt_blist(user_dir_t * user)
{
	struct list_head * l;
	bucket_t * b;
	printf("--------------- user -------------- %s\n",*(user->user_name));
	for_each_bucket(l,user){
		b = container_of(l,bucket_t,b_list);
		printf("%s - ",*(b->bucket_name));
	}
	printf("\n");
}
void prt_olist(bucket_t * bucket)
{
	struct list_head * l;
	object_t * o;
	printf("-------------- bucket ------------- %s\n",*(bucket->bucket_name));
	for_each_object(l,bucket){
		o = container_of(l,object_t,o_list);
		printf("%s - ",*(o->object_name));
	}
	printf("\n");
}
void prt_uhash(void)
{
	struct list_head * l;
	user_dir_t * u;
	int i;
	for(i=0;i<USER_HASH_NR;i++){
		if(list_empty(&user_hashtable[i])){
			continue;
		}
		printf(" -#%d- ",i);
		for_each_hash(l,&user_hashtable[i]){
			u = container_of(l,user_dir_t,u_hash);
			printf(": %s ",*(u->user_name));
		}
	}
	printf("\n");
	return;
}
void prt_bhash(void)
{
	struct list_head * l;
	bucket_t * b;
	int i;
	for(i=0;i<BKT_HASH_NR;i++){
		if(list_empty(&bucket_hashtable[i])){
			continue;
		}
		printf(" -#%d- ",i);
		for_each_hash(l,&bucket_hashtable[i]){
			b = container_of(l,bucket_t,b_hash);
			printf(": %s ",*(b->bucket_name));
		}
	}
	printf("\n");
	return;
}
void prt_ohash(void)
{
	struct list_head * l;
	object_t * o;
	int i;
	for(i=0;i<OBJ_HASH_NR;i++){
		if(list_empty(&object_hashtable[i])){
			continue;
		}
		printf(" -#%d- ",i);
		for_each_hash(l,&object_hashtable[i]){
			o = container_of(l,object_t,o_hash);
			printf(": %s ",*(o->object_name));
		}
	}
	printf("\n");
	return;
}
