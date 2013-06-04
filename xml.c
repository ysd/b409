#include"global.h"
#include"nss.h"
#include"list_head.h"
#include"xml.h"
static char buf[16];
int list_objects(bucket_t * bucket,char * xml_file)
{
	int rt = 0;
	struct list_head * l;
	object_t * object;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(bucket->bucket_name));
	xmlDocSetRootElement(doc,root_node);
	if(pthread_mutex_lock(&bucket->mutex) != 0){
		fprintf(stderr,"list objects lock bucket->mutex fail!\n");
		rt = 1;
		goto ret;
	}
	if(list_empty(&bucket->objects)){
		goto unlock_and_ret;
	}
	for_each_object(l,bucket){
		object = container_of(l,object_t,o_list);
		xmlNewChild(root_node,NULL,BAD_CAST *(object->object_name),BAD_CAST buf);
	}
unlock_and_ret:
	pthread_mutex_unlock(&bucket->mutex);
ret:
	xmlSaveFormatFileEnc(xml_file,doc,"UTF-8",1);
//	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return rt;
}
int list_buckets_objects(user_dir_t * user,char * xml_file,const char gu_flag)
{
	int rt = 0;
	struct list_head * l,*x;
	bucket_t * bucket;
	object_t * object;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL,bnode;
	if(!(GU_FLAG_VALID(gu_flag))){
		fprintf(stderr,"invalid get_user flag!\n");
		return 1;
	}
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(user->user_name));
	xmlDocSetRootElement(doc,root_node);
	if(pthread_mutex_lock(&user->mutex) != 0){
		perror("list_buckets_objects : pthread_mutex_lock u->mutex");
		rt = 2;
		goto ret;
	}
	if(list_empty(&user->buckets)){
		goto unlock_and_ret;
	}
	for_each_bucket(l,user){
		bucket = container_of(l,bucket_t,b_list);
		bnode = xmlNewChild(root_node,NULL,BAD_CAST *(bucket->bucket_name),BAD_CAST buf);
		if(gu_flag == GU_LIST_BUCKETS){
			continue;
		}
		if(pthread_mutex_lock(&bucket->mutex) != 0){
			fprintf(stderr,"lock bucket->mutex fail!\n");
			rt = 3;
			goto unlock_and_ret;
		}
		if(list_empty(&bucket->objects)){
			goto unlock_and_continue;
		}
		for_each_object(x,bucket){
			object = container_of(x,object_t,o_list);
			xmlNewChild(bnode,NULL,BAD_CAST *(object->object_name),BAD_CAST buf);
		}
unlock_and_continue:
		pthread_mutex_unlock(&bucket->mutex);
	}
unlock_and_ret:
	pthread_mutex_unlock(&user->mutex);
ret:
	xmlSaveFormatFileEnc(xml_file,doc,"UTF-8",1);
//	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return rt;
}
