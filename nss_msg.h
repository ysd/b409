#ifndef _NSS_MSG_H
#define _NSS_MSG_H
#include "global.h"
#include "name_buf.h"
extern int connect_to_nss_server(int *sockfd);
extern int put_object_msg_client(int sock_fd,char * object_name,char * bucket_name,char * user_name);
extern int get_object_msg_client(char * object_name,char * bucket_name,char * user_name);
extern int delete_object_msg_client(char * object_name,char * bucket_name,char * user_name);

extern int put_bucket_msg_client(int sock_fd, char * bucket_name,char * user_name);
extern int get_bucket_msg_client(char * bucket_name,char * user_name,char * xml_file);
extern int delete_bucket_msg_client(char * bucket_name,char * user_name);

extern int put_user_msg_client(char * user_name);
extern int get_user_msg_client(int sock_fd,char * user_name);
extern int delete_user_msg_client(char * user_name);

#endif
