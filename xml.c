#inlcude"global.h"
#include"nss.h"
#include"list_head.h"
#include"xml.h"
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
