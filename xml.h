#ifndef _XML_H
#define _XML_H
#include"nss.h"
#include<libxml/parser.h>
#include<libxml/tree.h>
#define ALL_BUCKETS_OBJECTS_FILE	"all.xml"
#define LIST_BUCKET_FILE			"b.xml"
#define LIST_OBJECT_FILE			"o.xml"
#define O_XML_HASH	"hash"
#define O_XML_ATIME	"st_atime"
#define O_XML_MTIME	"st_mtime"
#define O_XML_CTIME	"st_ctime"
#define O_XML_SIZE	"st_size"
#define O_XML_IP	"ip"
extern int list_objects(bucket_t * bucket,char * xml_file);
extern int list_buckets_objects(user_dir_t * user,char * xml_file,char gu_flag);
#endif
