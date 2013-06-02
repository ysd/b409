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
static void de_user_dir(user_dir_t * user)
{
	pthread_mutex_destroy(&user->mutex);
	put_name_zone(user->user_name);
	free(user);
	return;
}
static int get_user_dir_by_name(char * name,void ** ptr,const u8 op_style)
{
	/* search user_dir in user_hashtable
	 * synchronization is needed
	 * ---------------------------------
	 * RETURN VALUE : 
	 * 1) 0 ,not found,*ptr is set to be the address of list_head,
	 *	  before which new user should be inserted to 
	 * 2) 1 ,found,*ptr is set to be the address of correspondent user 
	 * 3) 2 ,error situation */
	int rt = 0,i,len = strlen(name),uhash = u_hash(name);
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
