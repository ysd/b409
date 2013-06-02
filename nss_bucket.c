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
static void de_bucket(bucket_t * bucket)
{
	pthread_mutex_destroy(&bucket->mutex);
	put_name_zone(bucket->bucket_name);
	free(bucket);
	return;
}
static int get_bucket_by_name(char * name,user_dir_t * user,void ** ptr,const u8 op_style)
{
	int rt = 0,i,len = strlen(name),bhash = b_hash(name,user);
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
