#include"global.h"
#include"utility.h"
#include"list_head.h"
#include"name_buf.h"
#include"nss.h"
#define BITS_OF_INT		32
#define OBJ_HASH_BITS	12
#define BKT_HASH_BITS	11
#define USER_HASH_BITS	10
#define OBJ_HASH_NR		(1 << OBJ_HASH_BITS)
#define BKT_HASH_NR		(1 << BKT_HASH_BITS)
#define USER_HASH_NR	(1 << USER_HASH_BITS)
#define OP_WITH_LOCK		00
#define OP_WITHOUT_LOCK		01
/* protect three hash tables */
pthread_mutex_t u_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t o_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
struct list_head user_hashtable[USER_HASH_NR];
struct list_head bucket_hashtable[BKT_HASH_NR];
struct list_head object_hashtable[OBJ_HASH_NR];
root_dir root;
root_dir * root_ptr;
static inline int __hash(char * name,void * upper_dir,int hash_bits)
{
	unsigned int int_upper_d = (unsigned int)upper_dir;
	unsigned int seed = 131;
	unsigned int hash = 0;
	unsigned char * p = (unsigned char *)name;
	while(*p != '\0'){
		hash = hash * seed + (*p++);
	}
	hash += int_upper_d;
	return (hash >> (BITS_OF_INT - hash_bits));
}
static inline int o_hash(char * obj_name,bucket_t * bucket)
{
	return __hash(obj_name,(void*)bucket,OBJ_HASH_BITS);
}
static inline int b_hash(char * bucket_name,user_dir_t * user_dir)
{
	return __hash(bucket_name,(void*)user_dir,BKT_HASH_BITS);
}
static inline int u_hash(char * user_name)
{
	return __hash(user_name,(void*)root_ptr,USER_HASH_BITS);
}
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
	root_ptr = &root;
	pthread_mutex_init(&root_ptr->mutex,NULL);
	list_head_init(&root_ptr->user_dirs);
	init_hash_table(user_hashtable,USER_HASH_NR);
	init_hash_table(bucket_hashtable,BKT_HASH_NR);
	init_hash_table(object_hashtable,OBJ_HASH_NR);
	return;
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
static int get_user_dir_by_name(char * name,void ** ptr,const u8 op_style)
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
static int get_bucket_by_name(char * name,user_dir_t * user,void ** ptr,const u8 op_style)
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
static int get_object_by_name(char * name,bucket_t * bucket,void ** ptr,u8 op_style)
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
		add_user_to_ulist(user);
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
		add_bucket_to_blist(bucket);
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
	list_add_tail(&object->o_hash,l);
unlock_and_ret:
	if(pthread_mutex_unlock(&o_hash_mutex) != 0){
		perror("add object : unlock o_hash_mutex");
		rt = 1;
	}
	if(rt == 0){
		add_object_to_olist(object);
	}
ret:
	return rt;
}
static int del_object_from_olist(object_t * object,const u8 op_style)
{
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&object->bucket->mutex) != 0){
			perror("del object : lock obj->bucket->mutex");
			return 1;
		}
	}
	list_del(&object->o_list);
	/* re_init object->o_list so that it will not reference other objects in olist */
	list_head_init(&object->o_list);
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&object->bucket->mutex) != 0){
			perror("del object : unlock obj->bucket->mutex");
			return 1;
		}
	}
	return 0;
}
static int del_object_from_oht(object_t * object,const u8 op_style)
{
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&o_hash_mutex) != 0){
			perror("del object : lock o_hash_mutex");
			return 1;
		}
	}
	list_del(&object->o_hash);
	/* re_init object->o_hash so that it will not reference other objects in hashtable */
	list_head_init(&object->o_hash);
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&o_hash_mutex) != 0){
			perror("del object : unlock o_hash_mutex");
			return 1;
		}
	}
	return 0;
}
int del_object(object_t * object)
{
	/* first delete from object list */
	if(del_object_from_olist(object,OP_WITH_LOCK) != 0){
		return 1;
	}
	/*	THIS DEL_OBJECT OPRATION OUGHT TO BE ATOMIC,BUT IT'S NOT.
	 *	SO IT IS INTERRUPTED HERE,SOME UNDEFINED STATE OF THE DELETION COMES OUT.
	 *	FOR EXAMPLE,ANOTHER PROCESS WANTS TO ADD AN OBJECT WHO SHARES
	 *	NAME WITH CURRENT OBJECT IN THE SAME BUCKET,
	 *	THAT ADD_OBJECT OPERATION WILL FAIL,
	 *	BECAUSE THIS DELETION IS NOT COMPLETE,THE OBJECT IS STILL IN HASH TABLE.*/
	if(del_object_from_oht(object,OP_WITH_LOCK) != 0){
		return 1;
	}
	put_name_zone(object->object_name);
	free(object);
	return 0;
}
static int del_bucket_from_blist(bucket_t * bucket,const u8 op_style)
{
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&bucket->user_dir->mutex) != 0){
			perror("del bucket : lock bucket->user_dir->mutex");
			return 1;
		}
	}
	list_del(&bucket->b_list);
	list_head_init(&bucket->b_list);
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&bucket->user_dir->mutex) != 0){
			perror("del bucket : unlock bucket->user_dir->mutex");
			return 1;
		}
	}
	return 0;
}
static int del_bucket_from_bht(bucket_t * bucket,const u8 op_style)
{
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&b_hash_mutex) != 0){
			perror("del bucket : lock b_hash_mutex");
			return 1;
		}
	}
	list_del(&bucket->b_hash);
	list_head_init(&bucket->b_hash);
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&b_hash_mutex) != 0){
			perror("del bucket : unlock b_hash_mutex");
			return 1;
		}
	}
	return 0;
}
int del_bucket(bucket_t * bucket)
{
	/* free a bucket after :
	 * 1) it is removed from bucket_list
	 * 2) it is removed from bucket_hashtable
	 * 3) after all its objects has been deleted
	 * 4) its name_zone is put back
	 * 5) bucket->mutex is destroyed
	 * */
	object_t * o;
	struct list_head * l;
	/* delete bucket from bucket list */
	if(del_bucket_from_blist(bucket,OP_WITH_LOCK) != 0){
		return 1;
	}
	/* here this bucket is still available in hash table,
	 * which may cause some inconsistency,but not severe problem.*/
	/* delete bucket from bucket hash table */
	if(del_bucket_from_bht(bucket,OP_WITH_LOCK) != 0){
		return 1;
	}
	/* now this bucket thoroughly disappears from the namespace,
	 * BUT WHAT IF SOME OTHER PROCESS GET THIS BUCKET WHEN THIS DELETION IS IN UNDEFINED STATE?
	 * since we have bucket->mutex locked here.
	 * THERE SHOULD BE A GENERAL PRINCIPLE THAT EVERY OPERATION 
	 * WHICH MAY DO SOME MODIFICATION ON THE SHARED LIST SUCH AS B_LIST AND HASH_TABLE,
	 * SHOULD ACCUIRE MUTEX FIRST! */
	 /* next step,delete all its objects */
	if(pthread_mutex_lock(&bucket->mutex) != 0){
		return 1;
	}
	for_each_object(l,bucket){
		o = container_of(l,object_t,o_list);
		/* objects are deleted from olist */
		del_object_from_olist(o,OP_WITHOUT_LOCK);
	}
	if(pthread_mutex_unlock(&bucket->mutex) != 0){
		return 1;
	}
	/* lock o_hash_mutex and delete all objects from oht */
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		return 1;
	}
	for_each_object(l,bucket){
		o = container_of(l,object_t,o_list);
		del_object_from_oht(o,OP_WITHOUT_LOCK);
		/* DON'T FORGET TO COMPLETE THE DELETION OF OBJCTS
		 * BY PUTTING BACK THE NAME_ZONE AND FREE THE OBJECT */
		put_name_zone(o->object_name);
		free(o);
	}
	if(pthread_mutex_unlock(&o_hash_mutex) != 0){
		return 1;
	}
	pthread_mutex_destroy(&bucket->mutex);
	put_name_zone(bucket->bucket_name);
	free(bucket);
	return 0;
}
static int del_user_from_ulist(user_dir_t * user,const u8 op_style)
{
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&root_ptr->mutex) != 0){
			perror("del user : lock root_ptr->mutex");
			return 1;
		}
	}
	list_del(&user->u_list);
	list_head_init(&user->u_list);
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&root_ptr->mutex) != 0){
			perror("del user : unlock root_ptr->mutex");
			return 1;
		}
	}
	return 0;
}
static int del_user_from_uht(user_dir_t * user,const u8 op_style)
{
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&u_hash_mutex) != 0){
			perror("del user : lock u_hash_mutex");
			return 1;
		}
	}
	list_del(&user->u_hash);
	list_head_init(&user->u_hash);
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_unlock(&u_hash_mutex) != 0){
			perror("del user : unlock u_hash_mutex");
			return 1;
		}
	}
	return 0;
}
int del_user(user_dir_t * user)
{
	/* free a user_dir after : 
	 * 1) it is removed from user_hashtable
	 * 2) it is removed from user_list 
	 * 3) all its buckets has been deleted
	 * 4) its name is put back
	 * */
	bucket_t * b;
	struct list_head * l;
	if(del_user_from_ulist(user,OP_WITH_LOCK) != 0){
		return 1;
	}
	if(del_user_from_uht(user,OP_WITH_LOCK) != 0){
		return 1;
	}
	for_each_bucket(l,user){
		b = container_of(l,bucket_t,b_list);
		if(del_bucket(b) != 0){
			return 1;
		}
	}
	pthread_mutex_destroy(&user->mutex);
	put_name_zone(user->user_name);
	free(user);
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
static void prt_ulist(void)
{
	struct list_head * l;
	user_dir_t * u;
	printf("------- user_list : \n");
	for_each_user(l){
		u = container_of(l,user_dir_t,u_list);
		printf("%s\n",*(u->user_name));
	}
}
static void prt_blist(user_dir_t * user)
{
	struct list_head * l;
	bucket_t * b;
	printf("user -- %s\n",*(user->user_name));
	for_each_bucket(l,user){
		b = container_of(l,bucket_t,b_list);
		printf("%s\n",*(b->bucket_name));
	}
}
static void prt_olist(bucket_t * bucket)
{
	struct list_head * l;
	object_t * o;
	printf("bucket -- %s\n",*(bucket->bucket_name));
	for_each_object(l,bucket){
		o = container_of(l,object_t,o_list);
		printf("%s\n",*(o->object_name));
	}
}
/*
static int do_s3_request(char * path)
{
	int rt = 0;
	return rt;
}
*/
int main()
{
	init_name_space();
	add_user("super_user");
	add_user("super_useri1");
	add_user("super_user2");
	add_user("super_user");
	prt_ulist();
	return 0;
}
