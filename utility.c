#include"global.h"
#include"utility.h"
#include"nss.h"
#include"list_head.h"
#include<libxml/parser.h>
#include<libxml/tree.h>
int xml_for_list_bucket(user_dir_t * user)
{
	struct list_head * l;
	bucket_t * bucket;
//	char * dest_file;
	u8  buf[XML_BUFSZ];
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(user->user_name));
	xmlDocSetRootElement(doc,root_node);
	for_each_bucket(l,user){
		bucket = container_of(l,bucket_t,b_list);
		xmlNewChild(root_node,NULL,BAD_CAST *(bucket->bucket_name),BAD_CAST buf);
	}
//	xmlSaveFormatFileEnc(dest_file,doc,"UTF-8",1);
	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}
int xml_for_list_object(bucket_t * bucket)
{
	struct list_head * l;
	object_t * object;
//	char * dest_file;
	u8  buf[XML_BUFSZ];
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(bucket->bucket_name));
	xmlDocSetRootElement(doc,root_node);
	for_each_object(l,bucket){
		object = container_of(l,object_t,o_list);
		xmlNewChild(root_node,NULL,BAD_CAST *(object->object_name),BAD_CAST buf);
	}
//	xmlSaveFormatFileEnc(dest_file,doc,"UTF-8",1);
	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}
void rand_generator(int a[],int len,int limit)
{
	int i;
	for(i = 0;i < len;i++){
		srand(time(NULL) + i);
		a[i] = rand()%limit;
	}
	return;
}
int get_cache_path(char* path,char* cache_path)
{
    char *p;
	bzero(cache_path,MAX_PATH);
    p = cache_path + strlen(CACHE_PATH);
    strcpy(cache_path,CACHE_PATH);
    strcpy(p,path);
    return 1;
}
int get_data_path(char* path,char* data_path)
{
    char *p;
	bzero(data_path,MAX_PATH);
    p = data_path + strlen(DATA_PATH);
    strcpy(data_path,DATA_PATH);
    strcpy(p,path);
    return 1;
}
int get_path_from_fd(int fd, char* path)
{
    char buf[1024];
	pid_t  pid;
    bzero(buf, 1024);
    pid = getpid();
	bzero(path,MAX_PATH);
    snprintf(buf, 1024, "/proc/%i/fd/%i", pid, fd);
    return readlink(buf, path, MAX_PATH);
}
