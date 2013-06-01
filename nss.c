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
void init_name_space(void)
{
	/* init namespace server */
	root_ptr = &root;
	pthread_mutex_init(&root_ptr->mutex,NULL);
	list_head_init(&root_ptr->user_dirs);
	init_hash_table(user_hashtable,USER_HASH_NR);
	init_hash_table(bucket_hashtable,BKT_HASH_NR);
	init_hash_table(object_hashtable,OBJ_HASH_NR);
	/* init name_buf */
	init_name_buf();
	GET_HASH_MASK(u_hashmask,USER_HASH_BITS);
	GET_HASH_MASK(b_hashmask,BKT_HASH_BITS);
	GET_HASH_MASK(o_hashmask,OBJ_HASH_BITS);
	return;
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
static user_dir_t * new_user_dir(char * user_name,u16 uid,u16 gid,u16 acl)
{
	/* make a new user_dir */
	int name_len = strlen(user_name);
	user_dir_t * new_user = (user_dir_t*)malloc(USER_DIR_SZ);
	if(new_user == NULL){
		goto ret;
	}
	/* 1) get_name_zone
	 * 2) init uga
	 * 3) init list_head
	 * 4) init_mutex */
	new_user->user_name = get_name_zone(name_len);
	strncpy(*(new_user->user_name),user_name,name_len);
	if(new_user->user_name == NAME_ZONE_NULL){
		goto free_new_user_and_ret;
	}
	set_uid(new_user,uid);
	set_gid(new_user,gid);
	set_acl(new_user,acl);
	list_head_init(&new_user->u_list);
	list_head_init(&new_user->u_hash);
	list_head_init(&new_user->buckets);
	pthread_mutex_init(&new_user->mutex,NULL);
	goto ret;
free_new_user_and_ret:
	free(new_user);
ret:
	return new_user;
}
static bucket_t * new_bucket(char * bucket_name,u16 uid,u16 gid,u16 acl,user_dir_t * user)
{
	/* make a new bucket */
	int name_len = strlen(bucket_name);
	bucket_t * newb = (bucket_t*)malloc(BUCKET_SZ);
	if(newb == NULL){
		goto ret;
	}
	/* 1) get_name_zone
	 * 2) init uga
	 * 3) init list_head
	 * 4) init_mutex 
	 * 5) set user_dir */
	newb->bucket_name = get_name_zone(name_len);
	strncpy(*(newb->bucket_name),bucket_name,name_len);
	if(newb->bucket_name == NAME_ZONE_NULL){
		goto free_new_bucket_and_ret;
	}
	set_uid(newb,uid);
	set_gid(newb,gid);
	set_acl(newb,acl);
	list_head_init(&newb->b_list);
	list_head_init(&newb->b_hash);
	list_head_init(&newb->objects);
	pthread_mutex_init(&newb->mutex,NULL);
	newb->user_dir = user;
	goto ret;
free_new_bucket_and_ret:
	free(newb);
ret:
	return newb;
}
static object_t * new_object(char * object_name,u16 uid,u16 gid,u16 acl,bucket_t * bucket)
{
	int name_len = strlen(object_name);
	object_t * newo = (object_t*)malloc(OBJECT_SZ);
	if(newo == NULL){
		goto ret;
	}
	/* 1) get_name_zone
	 * 2) init uga
	 * 3) init list_head
	 * 4) init_mutex 
	 * 5) set bucket */
	newo->object_name = get_name_zone(name_len);
	strncpy(*(newo->object_name),object_name,name_len);
	if(newo->object_name == NAME_ZONE_NULL){
		goto free_new_object_and_ret;
	}
	set_uid(newo,uid);
	set_gid(newo,gid);
	set_acl(newo,acl);
	list_head_init(&newo->o_list);
	list_head_init(&newo->o_hash);
	newo->bucket = bucket;
	goto ret;
free_new_object_and_ret:
	free(newo);
ret:
	return newo;
}
int get_user_dir_by_name(char * name,void ** ptr,const u8 op_style)
{
	/* search user_dir in user_hashtable
	 * synchronization is needed
	 * ---------------------------------
	 * RETURN VALUE : 
	 * 1) 0 ,found,*ptr is set to be the address of correspondent user 
	 * 2) 1 ,not found,*ptr is set to be the address of list_head,
	 *	  before which new user should be inserted to 
	 * 3) 2 ,error situation */
	int rt = 1,i,len = strlen(name),uhash = u_hash(name);
	user_dir_t * user;
	struct list_head * l;
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&u_hash_mutex) != 0){
			perror("lock u_hash_mutex");
			rt = 2;
			goto ret;
		}
	}
	for_each_hash(l,&user_hashtable[uhash]){
		user = container_of(l,user_dir_t,u_hash);
		i = strncmp(name,*(user->user_name),len);
		if(i == 0){
			/* found */
			*ptr = (void*)user;
			rt = 0;
			goto unlock_and_ret;
		}else if(i < 0){
			/* not found
			 * new user should be inserted right before user */
			break;
		}
	}
	*ptr = (void*)l;
unlock_and_ret:
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&u_hash_mutex) != 0){
			perror("unlock u_hash_mutex");
			rt = 2;
		}
	}
ret:
	return rt;
}
int get_bucket_by_name(char * name,user_dir_t * user,void ** ptr,const u8 op_style)
{
	int rt = 1,i,len = strlen(name),bhash = b_hash(name,user);
	bucket_t * bucket;
	struct list_head * l;
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&b_hash_mutex) != 0){
			perror("lock b_hash_mutex");
			rt = 2;
			goto ret;
		}
	}
	for_each_hash(l,&bucket_hashtable[bhash]){
		bucket = container_of(l,bucket_t,b_hash);
		i = strncmp(name,*(bucket->bucket_name),len);
		if(i == 0 && bucket->user_dir == user){
			/* found */
			rt = 0;
			*ptr = (void*)bucket;
			goto unlock_and_ret;
		}else if(i < 0){
			break;
		}
	}
	*ptr = (void*)l;
unlock_and_ret:
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&b_hash_mutex) != 0){
			perror("unlock b_hash_mutex");
			rt = 2;
		}
	}
ret:
	return rt;
}
int get_object_by_name(char * name,bucket_t * bucket,void ** ptr,u8 op_style)
{
	int i,rt = 1,len = strlen(name),ohash = o_hash(name,bucket);
	object_t * object;
	struct list_head * l;
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&o_hash_mutex) != 0){
			perror("lock o_hash_mutex");
			rt = 2;
			goto ret;
		}
	}
	for_each_hash(l,&object_hashtable[ohash]){
		object = container_of(l,object_t,o_hash);
		i = strncmp(name,*(object->object_name),len);
		if(i == 0 && object->bucket == bucket){
			*ptr = (void*)object;
			rt = 0;
			goto unlock_and_ret;
		}else if(i < 0){
			break;
		}
	}
	*ptr = (void*)l;
unlock_and_ret:
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&o_hash_mutex) != 0){
			perror("unlock o_hash_mutex");
			rt = 2;
		}
	}
ret:
	return rt;
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
	struct list_head * l;
	user_dir_t * u;
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
	if(pthread_mutex_unlock(&root_ptr->mutex) != 0){
		perror("add user to ulist : unlock root_ptr->mutex");
		return 2;
	}
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
	if(pthread_mutex_unlock(&bucket->user_dir->mutex) != 0){
		perror("add bucket to blist : unlock bucket->user_dir->mutex");
		return 2;
	}
	return 0;
}
static int add_object_to_olist(object_t * object)
{
	int rt = 0;
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
	if(pthread_mutex_unlock(&object->bucket->mutex) != 0){
		perror("add object to olist : unlock object->bucket->mutex");
		return 2;
	}
	return 0;
}
/* add_user : This api is provided for application to add new user to the system */
int add_user(char * user_name)
{
	/* 1) check if user with same name has already exist 
	 * 2) if not,get a new user_dir_t for new user
	 * 3) add to the user list and user hash table */
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
	if(get_user_dir_by_name(user_name,&ul,OP_WITHOUT_LOCK) == 0){
		fprintf(stderr,"user '%s' already exists!\n",user_name);
		rt = 2;
		goto unlock_and_ret;
	}
	l = (struct list_head *)ul;
//	uid = get_new_uid();
	uid = 0;
	gid = uid;
	acl = 0740;
	user = new_user_dir(user_name,uid,gid,acl);
	/* add to u_hashtable */
	list_add_tail(&user->u_hash,l);
unlock_and_ret:
	if(pthread_mutex_unlock(&u_hash_mutex) != 0){
		perror("add user unlock u_hash_mutex");
		rt = 1;
	}
	if(rt == 0){
		/* add to user hash table succeeds! */
		rt = add_user_to_ulist(user);
	}
ret:
	return rt;
}
int add_bucket(char * bucket_name,user_dir_t * user)
{
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
	if(get_bucket_by_name(bucket_name,user,&bl,OP_WITHOUT_LOCK) == 0){
		fprintf(stderr,"bucket '%s' already exists!\n",bucket_name);
		rt = 2;
		goto unlock_and_ret;
	}
	l = (struct list_head *)bl;
	uid = get_uid(user);
	gid = get_gid(user);
	acl = get_acl(user);
	bucket = new_bucket(bucket_name,uid,gid,acl,user);
	list_add_tail(&bucket->b_hash,l);
unlock_and_ret:
	if(pthread_mutex_unlock(&b_hash_mutex) != 0){
		perror("add bucket : unlock b_hash_mutex");
		rt = 1;
	}
	if(rt == 0){
		rt = add_bucket_to_blist(bucket);
	}
ret:
	return rt;
}
int add_object(char * object_name,bucket_t * bucket)
{
	int rt = 0;
	u16 uid,gid,acl;
	object_t * object;
	struct list_head * l;
	void * ol;
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		perror("add object : lock o_hash_mutex");
		rt = 1;
		goto ret;
	}
	if(get_object_by_name(object_name,bucket,&ol,OP_WITHOUT_LOCK) == 0){
		fprintf(stderr,"object '%s' already exists!\n",object_name);
		rt = 2;
		goto unlock_and_ret;
	}
	l = (struct list_head *)ol;
	uid = get_uid(bucket);
	gid = get_gid(bucket);
	acl = get_acl(bucket);
	object = new_object(object_name,uid,gid,acl,bucket);
	/* add to hash table */
	list_add_tail(&object->o_hash,l);
unlock_and_ret:
	if(pthread_mutex_unlock(&o_hash_mutex) != 0){
		perror("add object : unlock o_hash_mutex");
		rt = 1;
	}
	if(rt == 0){
		/* add to object list */
		rt = add_object_to_olist(object);
	}
ret:
	return rt;
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
static inline void simple_put_object_name_back_and_free_object(object_t * object)
{
	put_name_zone(object->object_name);
	free(object);
}
int del_object(object_t * object)
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
	simple_put_object_name_back_and_free_object(object);
	return 0;
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
static inline void simple_put_bucket_name_zone_back_and_destory_mutex_and_free_bucket(bucket_t * bucket)
{
	put_name_zone(bucket->bucket_name);
	pthread_mutex_destroy(&bucket->mutex);
	free(bucket);
}
int del_bucket(bucket_t * bucket)
{
	/* delete bucket from bucket list */
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
	simple_put_bucket_name_zone_back_and_destory_mutex_and_free_bucket(bucket);
	return 0;
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
		b = container_of(l,bucket_t,b_hash);
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
static inline void simple_put_user_name_zone_back_and_destroy_mutex_and_free_user(user_dir_t * user)
{
	put_name_zone(user->user_name);
	pthread_mutex_destroy(&user->mutex);
	free(user);
}
#define DBGMSG
int del_user(user_dir_t * user)
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
	atomic_del_buckets(user);
#ifdef DBGMSG
	printf("buckets in user deleted completely!\n");
#endif
	simple_put_user_name_zone_back_and_destroy_mutex_and_free_user(user);
#ifdef DBGMSG
	printf("user  totally removed!\n");
#endif
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
