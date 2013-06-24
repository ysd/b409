#include"global.h"
#include"list_head.h"
#include"name_buf.h"
#include"nss.h"
#include"md_type.h"
/* protect three hash tables */
pthread_mutex_t u_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t o_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
struct list_head user_hashtable[USER_HASH_NR];
struct list_head bucket_hashtable[BKT_HASH_NR];
struct list_head object_hashtable[OBJ_HASH_NR];
root_dir root;
root_dir * root_ptr;
static inline void init_hash_table(struct list_head * ht,int size)
{
	int i;
	for(i=0;i<size;i++){
		list_head_init(&ht[i]);
	}
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
	return __hash(obj_name,(void*)bucket,O_HASH_MASK);
}
static inline int b_hash(char * bucket_name,user_dir_t * user_dir)
{
	return __hash(bucket_name,(void*)user_dir,B_HASH_MASK);
}
static inline int u_hash(char * user_name)
{
	return __hash(user_name,(void*)root_ptr,U_HASH_MASK);
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
void get_absolute_path_of_object(object_t * object,char name_buf[])
{
	bzero(name_buf,MAX_PATH);
	snprintf(name_buf,MAX_PATH,ABS_PATH_FMT,*(object->bucket->user_dir->user_name),\
			*(object->bucket->bucket_name),*(object->object_name));
}
static object_t * new_object(char * object_name,u16 uid,u16 gid,u16 acl,bucket_t * bucket)
{
	int name_len = strlen(object_name);
	object_t * newo = (object_t*)malloc(OBJECT_SZ);
	if(newo == NULL){
		goto ret;
	}
	bzero(newo,OBJECT_SZ);
	newo->object_name = get_name_zone(name_len);
	if(newo->object_name == NAME_ZONE_NULL){
		goto free_new_object_and_ret;
	}
	strncpy(*(newo->object_name),object_name,name_len);
	set_uid(newo,uid);
	set_gid(newo,gid);
	set_acl(newo,acl);
	list_head_init(&newo->o_list);
	list_head_init(&newo->o_hash);
	newo->bucket = bucket;
	goto ret;
free_new_object_and_ret:
	free(newo);
	newo = NULL;
ret:
	return newo;
}
static bucket_t * new_bucket(char * bucket_name,u16 uid,u16 gid,u16 acl,user_dir_t * user)
{
	int name_len = strlen(bucket_name);
	bucket_t * newb = (bucket_t*)malloc(BUCKET_SZ);
	if(newb == NULL){
		goto ret;
	}
	bzero(newb,BUCKET_SZ);
	if(pthread_mutex_init(&newb->mutex,NULL) != 0){
		perror("initializing new_bucket->mutex");
		goto free_new_bucket_and_ret;
	}
	newb->bucket_name = get_name_zone(name_len);
	if(newb->bucket_name == NAME_ZONE_NULL){
		goto destroy_mutex_and_go_on;
	}
	strncpy(*(newb->bucket_name),bucket_name,name_len);
	set_uid(newb,uid);
	set_gid(newb,gid);
	set_acl(newb,acl);
	list_head_init(&newb->b_list);
	list_head_init(&newb->b_hash);
	list_head_init(&newb->objects);
	newb->user_dir = user;
	goto ret;
destroy_mutex_and_go_on:
	pthread_mutex_destroy(&newb->mutex);
free_new_bucket_and_ret:
	free(newb);
	newb = NULL;
ret:
	return newb;
}
static user_dir_t * new_user_dir(char * user_name,u16 uid,u16 gid,u16 acl)
{
	int name_len = strlen(user_name);
	user_dir_t * new_user = (user_dir_t*)malloc(USER_DIR_SZ);
	if(new_user == NULL){
		goto ret;
	}
	bzero(new_user,USER_DIR_SZ);
	if(pthread_mutex_init(&new_user->mutex,NULL) != 0){
		perror("initializing user->mutex");
		goto free_new_user_and_ret;
	}
	new_user->user_name = get_name_zone(name_len);
	if(new_user->user_name == NAME_ZONE_NULL){
		goto destroy_mutex_and_go_on;
	}
	strncpy(*(new_user->user_name),user_name,name_len);
	set_uid(new_user,uid);
	set_gid(new_user,gid);
	set_acl(new_user,acl);
	list_head_init(&new_user->u_list);
	list_head_init(&new_user->u_hash);
	list_head_init(&new_user->buckets);
	goto ret;
destroy_mutex_and_go_on:
	pthread_mutex_destroy(&new_user->mutex);
free_new_user_and_ret:
	free(new_user);
	new_user = NULL;
ret:
	return new_user;
}
static inline void de_object(object_t * object)
{
	put_name_zone(object->object_name);
	free(object);
}
static inline void de_bucket(bucket_t * bucket)
{
	pthread_mutex_destroy(&bucket->mutex);
	put_name_zone(bucket->bucket_name);
	free(bucket);
}
static inline void de_user_dir(user_dir_t * user)
{
	pthread_mutex_destroy(&user->mutex);
	put_name_zone(user->user_name);
	free(user);
}
/* get object from object hash table */
int get_object_by_name(char * name,bucket_t * bucket,void ** ptr,const u8 op_style)
{
	/* return value:
	 * 1) 0 : no object found,but the list_head_ptr 
	 *		  before which the new object should be inserted is stored in *ptr
	 * 2) 1 : get object success,object_ptr is stored in *ptr 
	 * 3) 2 : get object fail for some errors */
	int i,rt = 0,len,ohash;
	object_t * object;
	struct list_head * l;
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&o_hash_mutex) != 0){
			perror("lock o_hash_mutex");
			rt = 2;
			goto ret;
		}
	}
	len = strlen(name);
	ohash = o_hash(name,bucket);
	for_each_hash(l,&object_hashtable[ohash]){
		object = container_of(l,object_t,o_hash);
		i = strncmp(name,*(object->object_name),len);
		if(i == 0 && object->bucket == bucket){
			*ptr = (void*)object;
			rt = 1;
			goto unlock_and_ret;
		}else if(i < 0){
			break;
		}
	}
	*ptr = (void*)l;
unlock_and_ret:
	if(op_style == OP_WITH_LOCK){
		pthread_mutex_unlock(&o_hash_mutex);
	}
ret:
	return rt;
}
int get_bucket_by_name(char * name,user_dir_t * user,void ** ptr,const u8 op_style)
{
	int rt = 0,i,len,bhash;
	bucket_t * bucket;
	struct list_head * l;
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&b_hash_mutex) != 0){
			perror("lock b_hash_mutex");
			rt = 2;
			goto ret;
		}
	}
	len = strlen(name);
	bhash = b_hash(name,user);
	for_each_hash(l,&bucket_hashtable[bhash]){
		bucket = container_of(l,bucket_t,b_hash);
		i = strncmp(name,*(bucket->bucket_name),len);
		if(i == 0 && bucket->user_dir == user){
			/* found */
			rt = 1;
			*ptr = (void*)bucket;
			goto unlock_and_ret;
		}else if(i < 0){
			break;
		}
	}
	*ptr = (void*)l;
unlock_and_ret:
	if(op_style == OP_WITH_LOCK){
		pthread_mutex_unlock(&b_hash_mutex);
	}
ret:
	return rt;
}
int get_user_dir_by_name(char * name,void ** ptr,const u8 op_style)
{
	int rt = 0,i,len,uhash;
	user_dir_t * user;
	struct list_head * l;
	if(op_style == OP_WITH_LOCK){
		if(pthread_mutex_lock(&u_hash_mutex) != 0){
			perror("lock u_hash_mutex");
			rt = 2;
			goto ret;
		}
	}
	len = strlen(name);
	uhash = u_hash(name);
	for_each_hash(l,&user_hashtable[uhash]){
		user = container_of(l,user_dir_t,u_hash);
		i = strncmp(name,*(user->user_name),len);
		if(i == 0){
			/* found */
			*ptr = (void*)user;
			rt = 1;
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
		pthread_mutex_unlock(&u_hash_mutex);
	}
ret:
	return rt;
}
static int add_object_to_olist(object_t * object)
{
	object_t * o;
	struct list_head * l;
	int len;
	if(pthread_mutex_lock(&object->bucket->mutex) != 0){
		perror("add object to olist : lock object->bucket->mutex");
		return 1;
	}
	len = strlen(*(object->object_name));
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
static int add_bucket_to_blist(bucket_t * bucket)
{
	bucket_t * b;
	struct list_head * l;
	int len;
	if(pthread_mutex_lock(&bucket->user_dir->mutex) != 0){
		perror("add bucket to blist : lock bucket->user_dir->mutex");
		return 1;
	}
	len = strlen(*(bucket->bucket_name));
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
static int add_user_to_ulist(user_dir_t * user)
{
	user_dir_t * u;
	struct list_head * l;
	int len;
	if(pthread_mutex_lock(&root_ptr->mutex) != 0){
		perror("add user to ulist : lock root_ptr->mutex");
		return 1;
	}
	len = strlen(*(user->user_name));
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
int get_object(char * object_name,char * bucket_name,char * user_name)
{
	/* retrieve object 
	 * 1) check if this object exists
	 * 2) do some access control check
	 * 0 is returned if everything is ok,
	 * non-zero is returned if any error occurs
	 */
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	void * ol;
	if(get_user_dir_by_name(user_name,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_user_dir_by_name fail!\n");
		rt = 1;
		goto ret;
	}
	/* what if this user is deleted here?
	 * 1) get_bucket_by_name may fail if bucket is deleted too
	 * 2) get_bucket_by_name may not fail if bucket has not been deleted.
	 *    if so,we can still get the bucket which may be deleted soon.
	 *    here we just assume that the bucket is available */
	u = (user_dir_t*)ol;
	if(get_bucket_by_name(bucket_name,u,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_bucket_by_name fail!\n");
		rt = 2;
		goto ret;
	}
	/* what if this bucket is deleted here? 
	 * just as we talked above,if the get_object_by_name succeeds,
	 * than get_object will be successful,otherwise get_object will fail. */
	b = (bucket_t*)ol;
	if(get_object_by_name(object_name,b,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_object_by_name fail!\n");
		rt = 3;
		goto ret;
	}
	/* what if the object is deleted here?
	 * get_object will still be successful.
	 * but in the operation on this object such as getting the real data of the object may fail,
	 * if the real object is deleted! */
	o = (object_t*)ol;
	/* do some access control check here */
	/* here we just print the object name to notify that get_object is ok */
	printf("oname #%s\n",*(o->object_name));
	/* next step is to transfer object */
ret:
	return rt;
}
int put_object(char * object_name,char * bucket_name,char * user_name)
{
	/* 1) add to object hash table 
	 * 2) add to object list */
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	u16 uid,gid,acl;
	struct list_head * l;
	void * ol;
	if(get_user_dir_by_name(user_name,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_user_dir_by_name fail!\n");
		rt = 1;
		goto ret;
	}
	u = (user_dir_t*)ol;
	if(get_bucket_by_name(bucket_name,u,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_bucket_by_name fail!\n");
		rt = 2;
		goto ret;
	}
	b = (bucket_t*)ol;
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		fprintf(stderr,"get_bucket_by_name fail!\n");
		rt = 3;
		goto ret;
	}
	rt = get_object_by_name(object_name,b,&ol,OP_WITHOUT_LOCK);
	if(rt == 2){
		rt = 4;
		goto unlock_and_ret;
	}
	if(rt == 1){
		rt = 0;
		goto unlock_and_ret;
	}
	/* add new object to namespace */
	l = (struct list_head *)ol;
	uid = get_uid(b);
	gid = get_gid(b);
	acl = get_acl(b);
	o = new_object(object_name,uid,gid,acl,b);
	if(o == NULL){
		rt = 5;
		goto unlock_and_ret;
	}
	/* add to hash table */
	list_add_tail(&o->o_hash,l);
unlock_and_ret:
	pthread_mutex_unlock(&o_hash_mutex);
	/* add to olist */
	if(rt == 0){
		rt = add_object_to_olist(o);
	}
ret:
	return rt;
}
int delete_object(char * object_name,char * bucket_name,char * user_name)
{
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	struct list_head * l;
	void * ol;
	if(get_user_dir_by_name(user_name,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_user_dir_by_name fail!\n");
		rt = 1;
		goto ret;
	}
	u = (user_dir_t*)ol;
	if(get_bucket_by_name(bucket_name,u,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"get_bucket_by_name fail!\n");
		rt = 2;
		goto ret;
	}
	b = (bucket_t*)ol;
	if(pthread_mutex_lock(&o_hash_mutex) != 0){
		perror("del object : lock o_hash_mutex");
		rt = 3;
		goto ret;
	}
	rt = get_object_by_name(object_name,b,&ol,OP_WITHOUT_LOCK);
	if(rt == 2){
		fprintf(stderr,"get_object_by_name fail!\n");
		rt = 4;
		goto unlock_and_ret;
	}
	if(rt == 0){
		/* object not exist */
		fprintf(stderr,"object not exist!\n");
		rt = 5;
		goto unlock_and_ret;
	}
	rt = 0;
	o = (object_t*)ol;
	simple_del_object_from_oht(o);
unlock_and_ret:
	pthread_mutex_unlock(&o_hash_mutex);
	if(rt != 0){
		goto ret;
	}
	if(pthread_mutex_lock(&o->bucket->mutex) != 0){
		perror("del object : lock obj->bucket->mutex");
		rt = 6;
		goto ret;
	}
	simple_del_object_from_olist(o);
	pthread_mutex_unlock(&o->bucket->mutex);
	de_object(o);
ret:
	return rt;
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
		de_object(o);
	}
	if(pthread_mutex_unlock(&bucket->mutex) != 0){
		perror("del all objects in bucket : unlock bucket->mutex");
		return 1;
	}
	return 0;
}
int get_bucket(char * bucket_name,char * user_name,char * xml_file)
{
	/* list all the objects in bucket */
	/* do not use mutex */
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	u16 uid,gid,acl;
	struct list_head * l;
	void * ol;
	if(get_user_dir_by_name(user_name,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"user not exist!\n");
		rt = 1;
		goto ret;
	}
	u = (user_dir_t*)ol;
	if(get_bucket_by_name(bucket_name,u,&ol,OP_WITH_LOCK) != 1){
		fprintf(stderr,"bucket not exist!\n");
		rt = 2;
		goto ret;
	}
	b = (bucket_t*)ol;
	/* make a xml file which consists of all the objects of this bucket */
	if(list_objects(b,xml_file) != 0){
		fprintf(stderr,"list_objects fail!\n");
		rt = 3;
	}
ret:
	return rt;
}
int put_bucket(char * bucket_name,char * user_name)
{
	/* put a new bucket */
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	u16 uid,gid,acl;
	struct list_head * l;
	void * bl;
	if(get_user_dir_by_name(user_name,&bl,OP_WITH_LOCK) != 1){
		fprintf(stderr,"user not exist!\n");
		rt = 1;
		goto ret;
	}
	u = (user_dir_t*)bl;
	if(pthread_mutex_lock(&b_hash_mutex) != 0){
		perror("add bucket : lock b_hash_mutex");
		rt = 2;
		goto ret;
	}
	rt = get_bucket_by_name(bucket_name,u,&bl,OP_WITHOUT_LOCK);
	if(rt == 1){
		/* already exist */
		rt = 3;
		goto unlock_and_ret;
	}
	if(rt == 2){
		goto unlock_and_ret;
	}
	l = (struct list_head *)bl;
	uid = get_uid(u);
	gid = get_gid(u);
	acl = get_acl(u);
	b = new_bucket(bucket_name,uid,gid,acl,u);
	if(b == NULL){
		rt = 4;
		goto unlock_and_ret;
	}
	list_add_tail(&b->b_hash,l);
unlock_and_ret:
	pthread_mutex_unlock(&b_hash_mutex);
	if(rt == 0){
		rt = add_bucket_to_blist(b);
	}
ret:
	return rt;
}
int delete_bucket(char * bucket_name,char * user_name)
{
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	struct list_head * l;
	void * bl;
	if(get_user_dir_by_name(user_name,&bl,OP_WITH_LOCK) != 1){
		fprintf(stderr,"user not exist!\n");
		rt = 1;
		goto ret;
	}
	u = (user_dir_t*)bl;
	if(pthread_mutex_lock(&b_hash_mutex) != 0){
		perror("add bucket : lock b_hash_mutex");
		rt = 2;
		goto ret;
	}
	rt = get_bucket_by_name(bucket_name,u,&bl,OP_WITHOUT_LOCK);
	if(rt != 1){
		rt = 3;
		goto unlock_and_ret;
	}
	b = (bucket_t*)bl;
	simple_del_bucket_from_bht(b);
unlock_and_ret:
	pthread_mutex_unlock(&b_hash_mutex);
	if(rt != 0){
		goto ret;
	}
	if(pthread_mutex_lock(&b->user_dir->mutex) != 0){
		perror("del bucket : lock bucket->user_dir->mutex");
		rt = 4;
		goto ret;
	}
	simple_del_bucket_from_blist(b);
	pthread_mutex_unlock(&b->user_dir->mutex); 
	/* delete all its objects after it is deleted from bht&blist */
	atomic_del_objects(b);
	/* put back name zone & destory mutex & free bucket */
	de_bucket(b);
ret:
	return rt;
}
static int atomic_del_buckets(user_dir_t * user)
{
	bucket_t * b;
	struct list_head * l,*n;
	if(list_empty(&user->buckets)){
		return 0;
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
		de_bucket(b);
	}
	return 0;
}
int get_user(char * user_name,char * xml_file,char gu_flag)
{
	/* list bucket */
	int rt = 0;
	user_dir_t * u;
	bucket_t * b;
	struct list_head * l;
	void * bl;
	if(get_user_dir_by_name(user_name,&bl,OP_WITH_LOCK) != 1){
		fprintf(stderr,"user not exist!\n");
		rt = 1;
		goto ret;
	}
	u = (user_dir_t*)bl;
	if(list_buckets_objects(u,xml_file,gu_flag) != 0){
		fprintf(stderr,"list_buckets_objects fail!\n");
		rt = 2;
	}
ret:
//	return 0;
	return rt;
}
int put_user(char * user_name)
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
/*
#define DBGMSG
int delete_user(char * user_name)
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
void parse_path(char *full_name,char **user_name,char **bucket_name,char **obj_name)
{

    char *p=full_name;

	while(*p=='/'){
		p++;
	}
	*user_name = p;
	while(*p!='/'){
		p++;
	}
	*(p++) = '\0';

	while(*p=='/'){
		p++;
	}
	*bucket_name = p;
	while(*p!='/'){
		p++;
	}
	*(p++) = '\0';
	while(*p=='/'){
		p++;
	}
	*obj_name = p;
	return;
}
int init_md_of_obj(char *md5s)
{
	Meta_Data md;
	time_t t = time(NULL);
	bzero(&md,MD_SZ);
	md.atime = t;
	md.ctime = t;
	md.mtime = t;
	md.size = 1024;
	strcpy(md.replica[0].rep_ip,"192.168.0.244");
	if(md_put(md5s,&md) != 0){
		return 1;
	}
	return 0;
}
