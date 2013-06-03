#include"global.h"
#include"nss.h"
#include"list_head.h"
#include"xml.h"
#include<libxml/parser.h>
#include<libxml/tree.h>
static char * buf = "test";
/*
int xml_for_list_bucket(user_dir_t * user,char * xml_file)
{
	struct list_head * l;
	bucket_t * bucket;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(user->user_name));
	xmlDocSetRootElement(doc,root_node);
	for_each_bucket(l,user){
		bucket = container_of(l,bucket_t,b_list);
		xmlNewChild(root_node,NULL,BAD_CAST *(bucket->bucket_name),BAD_CAST buf);
	}
	xmlSaveFormatFileEnc(xml_file,doc,"UTF-8",1);
//	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}
int xml_for_list_object(bucket_t * bucket,char * xml_file)
{
	struct list_head * l;
	object_t * object;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(bucket->bucket_name));
	xmlDocSetRootElement(doc,root_node);
	for_each_object(l,bucket){
		object = container_of(l,object_t,o_list);
		xmlNewChild(root_node,NULL,BAD_CAST *(object->object_name),BAD_CAST buf);
	}
	xmlSaveFormatFileEnc(xml_file,doc,"UTF-8",1);
//	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}
*/
int list_all_buckets_and_objects(user_dir_t * user,char * xml_file)
{
	struct list_head * l,*x;
	bucket_t * bucket;
	object_t * object;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL,bnode;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST *(user->user_name));
	xmlDocSetRootElement(doc,root_node);
	if(list_empty(&user->buckets)){
		goto ret;
	}
	for_each_bucket(l,user){
		bucket = container_of(l,bucket_t,b_list);
		bnode = xmlNewChild(root_node,NULL,BAD_CAST *(bucket->bucket_name),BAD_CAST buf);
		if(list_empty(&bucket->objects)){
			continue;
		}
		for_each_object(x,bucket){
			object = container_of(x,object_t,o_list);
			xmlNewChild(bnode,NULL,BAD_CAST *(object->object_name),BAD_CAST buf);
		}
	}
ret:
	xmlSaveFormatFileEnc(xml_file,doc,"UTF-8",1);
//	xmlSaveFormatFileEnc("-",doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}
