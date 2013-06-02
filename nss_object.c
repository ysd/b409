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
static void de_object(object_t * object)
{
	put_name_zone(object->object_name);
	free(object);
	return;
}
int get_object_by_name(char * name,bucket_t * bucket,void ** ptr,u8 op_style)
{
	int i,rt = 0,len = strlen(name),ohash = o_hash(name,bucket);
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
