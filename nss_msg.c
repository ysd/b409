#include "nss_msg.h"
#include "nss.h"
#include "common.h"
int connect_to_nss_server(int *sock_fd)
{
	int sockfd;
	struct sockaddr_un servaddr;
	size_t addrLen;
	if((sockfd = socket(PF_UNIX,SOCK_STREAM,0))== -1)
	{
		printf("fail to creat socket");
		return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path,UNIXSTR_PATH);
	addrLen = sizeof(servaddr.sun_family)+strlen(servaddr.sun_path);
	if(connect(sockfd,(struct sockaddr *)&servaddr, addrLen) == -1)
	{
		printf("cannot connect to server\n");
		return -1;
	}
	*sock_fd=sockfd;
	return 0;
}
int put_object_msg_client(int sock_fd, char * object_name,char * bucket_name,char * user_name)
{
	int n;
	char buf[MAX_LENGTH];
	/*************************first write***************/
	snprintf(buf,MAX_LENGTH,"%s %s %s %s","PUTOBJECT",object_name,bucket_name,user_name);
	write(sock_fd,buf,strlen(buf));

	/*************************first read*************/
	n = read(sock_fd,buf,MAX_LENGTH);
	buf[n]='\0';
	if(n > 0)
	{
		printf("receive from nss_server:%s\n",buf);
		if(buf[0] == 'O')//返回字符串的第一个字符为OK中的O
			return 0;
	}
	else
	{
		printf("cannot receive from server\n");
		return -1;
	}
	return 0;
}

int get_object_msg_client(char * object_name,char * bucket_name,char * user_name)
{
	return 0;
}

int delete_object_msg_client(char * object_name,char * bucket_name,char * user_name)
{
	return 0;

}

int put_bucket_msg_client(int sock_fd, char * bucket_name,char * user_name)
{

	int n;
	char buf[MAX_LENGTH];
	/*************************first write***************/
	snprintf(buf,MAX_LENGTH,"%s %s %s","PUTBUCKET",bucket_name,user_name);
	write(sock_fd,buf,strlen(buf));

	/*************************first read*************/
	n = read(sock_fd,buf,MAX_LENGTH);
	buf[n]='\0';
	if(n > 0)
	{
		printf("receive from nss_server:%s\n",buf);
		if(buf[0] == 'O')//返回字符串的第一个字符为OK中的O
			return 0;
	}
	else
	{
		printf("cannot receive from server\n");
		return -1;
	}
	return 0;
}

int get_bucket_msg_client(char * bucket_name,char * user_name,char * xml_file)
{
	return 0;

}

int delete_bucket_msg_client(char * bucket_name,char * user_name)
{
	return 0;

}

int put_user_msg_client(char * user_name)
{
	return 0;

}

int get_user_msg_client(int sock_fd,char * user_name)
{
	int n;
	char buf[MAX_LENGTH];
	/*************************first write***************/
	snprintf(buf,MAX_LENGTH,"%s %s","GETUSER",user_name);
	write(sock_fd,buf,strlen(buf));

	/*************************first read*************/
	n = read(sock_fd,buf,MAX_LENGTH);
	buf[n]='\0';
	if(n > 0)
	{
		printf("receive from nss_server:%s\n",buf);
		if(buf[0] == 'O')//返回字符串的第一个字符为OK中的O
			return 0;
	}
	else
	{
		printf("cannot receive from server\n");
		return -1;
	}
	return 0;
}

int delete_user_msg_client(char * user_name)
{
	return 0;
}
