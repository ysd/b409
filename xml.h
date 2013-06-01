#ifndef _XML_H
#define _XML_H
_PROTOTYPE(int xml_for_list_bucket,(user_dir_t * user));
_PROTOTYPE(int xml_for_list_object,(bucket_t * bucket));
_PROTOTYPE(int parse_bucket_xml,(char * xml_file));
_PROTOTYPE(int parse_object_xml,(char * xml_file));
#endif
