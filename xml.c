#include"global.h"
#include"nss.h"
#include"list_head.h"
#include"xml.h"
#include"md_type.h"
#include"md5.h"
static char buf[BUFSIZ];
static char buf_null[2] = {0};
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
		xmlNewChild(root_node,NULL,BAD_CAST *(object->object_name),BAD_CAST buf_null);
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
	xmlNodePtr root_node = NULL,bnode,onode;
	Meta_Data md;
	char full_path[MAX_PATH];
	char md5c[MD5_CHECKSUM_SZ];
	char md5s[MD5_STRING_LEN];
	int full_path_len;
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
		bnode = xmlNewChild(root_node,NULL,BAD_CAST *(bucket->bucket_name),BAD_CAST buf_null);
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
			onode = xmlNewChild(bnode,NULL,BAD_CAST *(object->object_name),BAD_CAST buf_null);
	//		get_absolute_path_of_object(object,full_path);
	//		full_path_len = strlen(full_path);
	//		md5(full_path,full_path_len,md5c);
	//		bzero(md5s,MD5_STRING_LEN);
	//		md5_2_str(md5c,md5s);
	//		/* md5 string now in md5s */
	//		if(md_get(md5s,&md) != 0){
	//			continue;
	//		}
	//		xmlNewChild(onode,NULL,BAD_CAST O_XML_HASH,BAD_CAST md5s);
	//		u32_to_str(md.stat_info.st_atime,buf,BUFSIZ);
	//		xmlNewChild(onode,NULL,BAD_CAST O_XML_ATIME,BAD_CAST buf);
	//		u32_to_str(md.stat_info.st_mtime,buf,BUFSIZ);
	//		xmlNewChild(onode,NULL,BAD_CAST O_XML_MTIME,BAD_CAST buf);
	//		u32_to_str(md.stat_info.st_ctime,buf,BUFSIZ);
	//		xmlNewChild(onode,NULL,BAD_CAST O_XML_CTIME,BAD_CAST buf);
	//		u32_to_str(md.stat_info.st_size,buf,BUFSIZ);
	//		xmlNewChild(onode,NULL,BAD_CAST O_XML_SIZE,BAD_CAST buf);
	//		xmlNewChild(onode,NULL,BAD_CAST O_XML_IP,BAD_CAST md.replica[0].rep_ip);
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
