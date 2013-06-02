#include"global.h"
#include"list_head.h"
#include"name_buf.h"
/*
#include"xml.h"
*/
#include"nss.h"
/* protect three hash tables */
pthread_mutex_t u_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t o_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
struct list_head user_hashtable[USER_HASH_NR];
struct list_head bucket_hashtable[BKT_HASH_NR];
struct list_head object_hashtable[OBJ_HASH_NR];
root_dir root;
root_dir * root_ptr;
static int u_hashmask;
static int b_hashmask;
static int o_hashmask;
static inline void init_hash_table(struct list_head * ht,int size)
{
	int i;
	for(i=0;i<size;i++){
		list_head_init(&ht[i]);
	}
	return;
}
int init_name_space(void)
{
	/* init namespace server */
	root_ptr = &root;
	bzero(root_ptr,ROOT_DIR_SZ);
	if(pthread_mutex_init(&root_ptr->mutex,NULL) != 0){
		perror("initializing root_ptr->mutex");
		return 1;
	}
	list_head_init(&root_ptr->user_dirs);
	init_hash_table(user_hashtable,USER_HASH_NR);
	init_hash_table(bucket_hashtable,BKT_HASH_NR);
	init_hash_table(object_hashtable,OBJ_HASH_NR);
	/* init name_buf */
	init_name_buf();
	GET_HASH_MASK(u_hashmask,USER_HASH_BITS);
	GET_HASH_MASK(b_hashmask,BKT_HASH_BITS);
	GET_HASH_MASK(o_hashmask,OBJ_HASH_BITS);
	return 0;
}
static inline int __hash(char * name,void * upper_dir,int hash_mask)
{
	int int_upper_d = (int)upper_dir;
	int seed = 131;
	int hash = 0;
	char * p = name;
	while(*p != '\0'){
		hash = hash * seed + (*p++);
	}
	hash += int_upper_d;
	return (hash & hash_mask);
}
static inline int o_hash(char * obj_name,bucket_t * bucket)
{
	return __hash(obj_name,(void*)bucket,o_hashmask);
}
static inline int b_hash(char * bucket_name,user_dir_t * user_dir)
{
	return __hash(bucket_name,(void*)user_dir,b_hashmask);
}
static inline int u_hash(char * user_name)
{
	return __hash(user_name,(void*)root_ptr,u_hashmask);
}





/* list : 
 *
 *			1) users_list 
 *			2) buckets_list 
 *			3) objects_list
 *			4) hash_table_list
 *			
 *	are all in alphabetical order */
static int add_user_to_ulist(user_dir_t * user)
{
	/* atomic operation */
	user_dir_t * u;
	struct list_head * l;
	int len = strlen(*(user->user_name));
	if(pthread_mutex_lock(&root_ptr->mutex) != 0){
		perror("add user to ulist : lock root_ptr->mutex");
		return 1;
	}
	/* find the right place to insert to */
	for_each_user(l){
		u = container_of(l,user_dir_t,u_list);
		if(strncmp(*(user->user_name),*(u->user_name),len) < 0){
			break;
		}
	}
	list_add_tail(&user->u_list,l);
	pthread_mutex_unlock(&root_ptr->mutex);
	return 0;
}
static int add_bucket_to_blist(bucket_t * bucket)
{
	bucket_t * b;
	struct list_head * l;
	int len = strlen(*(bucket->bucket_name));
	if(pthread_mutex_lock(&bucket->user_dir->mutex) != 0){
		perror("add bucket to blist : lock bucket->user_dir->mutex");
		return 1;
	}
	for_each_bucket(l,bucket->user_dir){
		b = container_of(l,bucket_t,b_list);
		if(strncmp(*(bucket->bucket_name),*(b->bucket_name),len) < 0){
			break;
		}
	}
	list_add_tail(&bucket->b_list,l);
	pthread_mutex_unlock(&bucket->user_dir->mutex);
	return 0;
}
static int add_object_to_olist(object_t * object)
{
	object_t * o;
	struct list_head * l;
	int len = strlen(*(object->object_name));
	if(pthread_mutex_lock(&object->bucket->mutex) != 0){
		perror("add object to olist : lock object->bucket->mutex");
		return 1;
	}
	for_each_object(l,object->bucket){
		o = container_of(l,object_t,o_list);
		if(strncmp(*(object->object_name),*(o->object_name),len) < 0){
			break;
		}
	}
	list_add_tail(&object->o_list,l);
	pthread_mutex_unlock(&object->bucket->mutex);
	return 0;
}
static inline void simple_del_object_from_olist(object_t * object)
{
	list_del(&object->o_list);
	list_head_init(&object->o_list);
}
static inline void simple_del_object_from_oht(object_t * object)
{
	list_del(&object->o_hash);
	list_head_init(&object->o_hash);
}
static inline void simple_del_bucket_from_blist(bucket_t * bucket)
{
	list_del(&bucket->b_list);
	list_head_init(&bucket->b_list);
}
static inline void simple_del_bucket_from_bht(bucket_t * bucket)
{
	list_del(&bucket->b_hash);
	list_head_init(&bucket->b_hash);
}
static inline void simple_del_user_from_ulist(user_dir_t * user)
{
	list_del(&user->u_list);
	list_head_init(&user->u_list);
}
static inline void simple_del_user_from_uht(user_dir_t * user)
{
	list_del(&user->u_hash);
	list_head_init(&user->u_hash);
}
/* add_user : This api is provided for application to add new user to the system */
int add_user(char * user_name)
{
	int rt = 0;
	u16 uid,gid,acl;
	user_dir_t * user;
	struct list_head * l;
	void * ul;
	if(pthread_mutex_lock(&u_hash_mutex) != 0){
		perror("add user lock u_hash_mutex");
		rt = 1;
		goto ret;
	}
	rt = get_user_dir_by_name(user_name,&ul,OP_WITHOUT_LOCK);
	if(rt == 1){
		rt = 2;
		fprintf(stderr,"user '%s' already exists!\n",user_name);
		goto unlock_and_ret;
	}else if(rt == 2){
		rt = 3;
		fprintf(stderr,"error in get_user_dir_by_name!\n");
		goto unlock_and_ret;
	}
	l = (struct list_head *)ul;
//	uid = get_new_uid();
	uid = 0;
	gid = uid;
	acl = 0740;
	user = new_user_dir(user_name,uid,gid,acl);
	if(user == NULL){
		rt = 4;
		goto unlock_and_ret;
	}
	/* add to u_hashtable */
	list_add_tail(&user->u_hash,l);
unlock_and_ret:
	pthread_mutex_unlock(&u_hash_mutex);
	if(rt == 0){
		/* add to user hash table succeeds! */
		rt = add_user_to_ulist(user);
	}
ret:
	return rt;
}

#define DBGMSG
int del_user(char * user_name)
{
	bucket_t * b;
	struct list_head * l;
	if(pthread_mutex_lock(&root_ptr->mutex) != 0){
		perror("del user : lock root_ptr->mutex");
		return 1;
	}
	simple_del_user_from_ulist(user);
	if(pthread_mutex_unlock(&root_ptr->mutex) != 0){
		perror("del user : unlock root_ptr->mutex");
		return 1;
	}
#ifdef DBGMSG
	printf("user removed from u list!\n");
#endif
	if(pthread_mutex_lock(&u_hash_mutex) != 0){
		perror("del user : lock u_hash_mutex");
		return 1;
	}
	simple_del_user_from_uht(user);
	if(pthread_mutex_unlock(&u_hash_mutex) != 0){
		perror("del user : unlock u_hash_mutex");
		return 1;
	}
#ifdef DBGMSG
	printf("user removed from u ht!\n");
#endif
	if(atomic_del_buckets(user) != 0){
		fprintf(stderr,"atomic_del_buckets fail!\n");
		return 1;
	}
#ifdef DBGMSG
	printf("buckets in user deleted completely!\n");
#endif
	de_user_dir(user);
#ifdef DBGMSG
	printf("user  totally removed!\n");
#endif
	return 0;
}
int get_object(char * object_name,char * bucket_name,char * user_name)
{
	/* retrieve object 
	 * here just make sure the object really exists! */
	int rt = 0;
	return rt;
}
int put_object(char * object_name,char * bucket_name,char * user_name)
{
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	u16 uid,gid,acl;
	struct list_head * l;
	void * ol;
	if(get_user_dir_by_name(user_name,&ol,OP_WITH_LOCK) != 1){
		goto ret;
	}
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		perror("add object : lock o_hash_mutex");
		rt = 1;
		goto ret;
	}
	rt = get_object_by_name(object_name,bucket,&ol,OP_WITHOUT_LOCK);
	if(rt == 1){
		rt = 2;
		fprintf(stderr,"object '%s' already exists!\n",object_name);
		goto unlock_and_ret;
	}else if(rt == 2){
		rt = 3;
		fprintf(stderr,"error in get_object_by_name!\n");
		goto unlock_and_ret;
	}
	l = (struct list_head *)ol;
	uid = get_uid(bucket);
	gid = get_gid(bucket);
	acl = get_acl(bucket);
	o = new_object(object_name,uid,gid,acl,bucket);
	if(o == NULL){
		rt = 4;
		goto unlock_and_ret;
	}
	/* add to hash table */
	list_add_tail(&o->o_hash,l);
unlock_and_ret:
	pthread_mutex_unlock(&o_hash_mutex);
	if(rt == 0){
		/* add to object list */
		rt = add_object_to_olist(o);
	}
ret:
	return rt;
}
int delete_object(char * object_name,char * bucket_name,char * user_name)
{
	/* first delete from object list */
	if(pthread_mutex_lock(&object->bucket->mutex) != 0){
		perror("del object : lock obj->bucket->mutex");
		return 1;
	}
	simple_del_object_from_olist(object);
	if(pthread_mutex_unlock(&object->bucket->mutex) != 0){
		perror("del object : unlock obj->bucket->mutex");
		return 1;
	}
	/*	THIS DEL_OBJECT OPRATION OUGHT TO BE ATOMIC,BUT IT'S NOT HERE.
	 *  here object still exists in hash table,
	 *  so a new add_object with this object will fail.
	 *  that is to say,no re_add of the same object is allowed before it is compeletly deleted!
	 */
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		perror("del object : lock o_hash_mutex");
		return 1;
	}
	simple_del_object_from_oht(object);
	if(pthread_mutex_unlock(&o_hash_mutex) != 0){
		perror("del object : unlock o_hash_mutex");
		return 1;
	}
	de_object(object);
	return 0;
}
static int atomic_del_objects(bucket_t * bucket)
{
	/* called when deleting a whole bucket
	 * HOWEVER,BEFORE CALLING THIS FUNCTION IN DELETING A BUCKET,
	 * THIS BUCKET SHOULD HAVE ALREADY BEEN DELETE FROM BUCKET_HASH_TABEL,
	 * SO THAT NO OTHERS WILL OPERATE ON THIS BUCKET */
	object_t * o;
	struct list_head * l,*n;
	if(list_empty(&bucket->objects)){
		return 0;
	}
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		perror("del all objects in hash table : lock o_hash_mutex");
		return 1;
	}
	/* we can only delete objects from hash table first,
	 * because object list is the only way to reference all the objects in buckets.
	 * (as traversing the objects hash table is way too inefficient )*/
	for_each_object(l,bucket){
		o = container_of(l,object_t,o_list);
		simple_del_object_from_oht(o);
	}
	if(pthread_mutex_unlock(&o_hash_mutex) != 0){
		perror("del all objects in hash table : unlock o_hash_mutex");
		return 1;
	}
	/* if this bucket still in hash table,
	 * when this function is interrupted here,
	 * other threads get this bucket and operate on it,
	 * such as add an object A which has been deleted from oht 
	 * but still in olist of this bucket by codes above,
	 * but adding object A is still allowed,
	 * if this happened,object A will be put to hash table again
	 * and there will be two identical objects in olist,
	 * AS TO THE OLIST,ALL OBJECTS WILL BE REMOVEED BY THE FOLLOWING CODES,
	 * BUT AS TO THE OBJECTS HASH TABLE,NOT ALL OBJECTS ARE REMOVED,SUCH AS OBJECT a.
	 * all reference to the object A will cause a segmentation fault
	 * */
	/* SO,JUST MAKE SURE THAT THE BUCKET HAS BEEN COMPLETELY DELETED FROM BUCKET HASH TABLE AND BLIST BEFORE DELETING ALL ITS OBJECTS*/
	if(pthread_mutex_lock(&bucket->mutex) != 0){
		perror("del all objects in bucket : lock bucket->mutex");
		return 1;
	}
	n = bucket->objects.next;
	while(n != &bucket->objects){
		l = n;
		n = l->next;
		o = container_of(l,object_t,o_list);
		simple_del_object_from_olist(o);
		/* DON'T FORGET TO COMPLETE THE DELETION OF OBJCTS
		 * BY PUTTING BACK THE NAME_ZONE AND FREE THE OBJECT */
		simple_put_object_name_back_and_free_object(o);
	}
	if(pthread_mutex_unlock(&bucket->mutex) != 0){
		perror("del all objects in bucket : unlock bucket->mutex");
		return 1;
	}
	return 0;
}
int get_bucket(char * bucket,char * user_name)
{
	/* list all the objects in bucket */
	int rt = 0;
	return rt;
}
int put_bucket(char * bucket_name,char * user_name)
{
	/* put a new bucket */
	int rt = 0;
	u16 uid,gid,acl;
	bucket_t * bucket;
	struct list_head * l;
	void * bl;
	if(pthread_mutex_lock(&b_hash_mutex) != 0){
		perror("add bucket : lock b_hash_mutex");
		rt = 1;
		goto ret;
	}
	rt = get_bucket_by_name(bucket_name,user,&bl,OP_WITHOUT_LOCK);
	if(rt == 1){
		rt = 2;
		fprintf(stderr,"bucket '%s' already exists!\n",bucket_name);
		goto unlock_and_ret;
	}else if(rt == 2){
		rt = 3;
		fprintf(stderr,"error in get_bucket_by_name!\n");
		goto unlock_and_ret;
	}
	l = (struct list_head *)bl;
	uid = get_uid(user);
	gid = get_gid(user);
	acl = get_acl(user);
	bucket = new_bucket(bucket_name,uid,gid,acl,user);
	if(bucket == NULL){
		rt = 4;
		goto unlock_and_ret;
	}
	list_add_tail(&bucket->b_hash,l);
unlock_and_ret:
	pthread_mutex_unlock(&b_hash_mutex);
	if(rt == 0){
		rt = add_bucket_to_blist(bucket);
	}
ret:
	return rt;
}
int delete_bucket(char * bucket_name,char * user_name)
{
	/* delete all the objects and delete bucket */
	if(pthread_mutex_lock(&bucket->user_dir->mutex) != 0){
		perror("del bucket : lock bucket->user_dir->mutex");
		return 1;
	}
	simple_del_bucket_from_blist(bucket);
	if(pthread_mutex_unlock(&bucket->user_dir->mutex) != 0){
		perror("del bucket : unlock bucket->user_dir->mutex");
		return 1;
	}
	/* delete from hash table */
	if(pthread_mutex_lock(&b_hash_mutex) != 0){
		perror("del bucket : lock b_hash_mutex");
		return 1;
	}
	simple_del_bucket_from_bht(bucket);
	if(pthread_mutex_unlock(&b_hash_mutex) != 0){
		perror("del bucket : unlock b_hash_mutex");
		return 1;
	}
	/* delete all its objects after it is deleted from bht&blist */
	atomic_del_objects(bucket);
	/* put back name zone & destory mutex & free bucket */
	de_bucket(bucket);
	return 0;
}
static int atomic_del_buckets(user_dir_t * user)
{
	/* this function should be called after user has been deleted from hash table and ulist */
	bucket_t * b;
	struct list_head * l,*n;
	if(list_empty(&user->buckets)){
		return 0;
	}
	if(pthread_mutex_trylock(&user->mutex) != 0){
		perror("del all buckets in blist : lock user->mutex");
		return 1;
	}
	if(pthread_mutex_trylock(&b_hash_mutex) != 0){
		perror("del all buckets in hash table : lock b_hash_mutex");
		return 1;
	}
	for_each_bucket(l,user){
		b = container_of(l,bucket_t,b_list);
		simple_del_bucket_from_bht(b);
	}
	if(pthread_mutex_unlock(&b_hash_mutex) != 0){
		perror("del all buckets in hash table : unlock b_hash_mutex");
		return 1;
	}
	n = user->buckets.next;
	while(n != &user->buckets){
		l = n;
		n = l->next;
		b = container_of(l,bucket_t,b_list);
		/* delete bucket from bucket list */
		simple_del_bucket_from_blist(b);
		atomic_del_objects(b);
		simple_put_bucket_name_zone_back_and_destory_mutex_and_free_bucket(b);
	}
	if(pthread_mutex_unlock(&user->mutex) != 0){
		perror("del all buckets in blist : unlock user->mutex");
		return 1;
	}
	return 0;
}

void get_absolute_path_of_object(object_t * object,char name_buf[])
{
	bzero(name_buf,MAX_PATH);
	snprintf(name_buf,MAX_PATH,ABS_PATH_FMT,*(object->bucket->user_dir->user_name),\
			*(object->bucket->bucket_name),*(object->object_name));
	return;
}
/*
void list_bucket(user_dir_t * user)
{
	xml_for_list_bucket(user);
	return;
}
void list_object(bucket_t * bucket)
{
	xml_for_list_object(bucket);
	return;
}
*/
