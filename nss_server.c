#include"global.h"
#include"md_type.h"
#include"md5.h"
#include"utility.h"
#include"name_buf.h"
#include "common.h"
#include "nss.h"
#include "xml.h"
#include "common.h"
#include "command_split.h"
void * handle(void *arg)
{
	int k,m,i,j, connfd;
	char buf[MAX_LENGTH];
	struct command_line command;
	connfd = *((int *)arg);
	free(arg);
	//首先获取请求命令
	m = read(connfd,buf,MAX_LENGTH);
	buf[m]='\0';
	if(m<0)
	{
		printf("cannot get GETUSER\n");
		exit(-1);
	}
	printf("receive from server:%s\n",buf);

	//解析每个命令,并执行相应操作，操作正确完成，则向客户端返回"OK",否则返回“Error“
    if(strstr(buf, "GETUSER") ==buf)
	{
		j = split(&command,buf);
		if(j == 2)
		{
			i = get_user(command.argv[1],ALL_BUCKETS_OBJECTS_FILE,GU_LIST_ALL_BUCKETS_OBJECTS);
			if(i== -1)
			{	
				put_user(command.argv[1]);
				j=get_user(command.argv[1],ALL_BUCKETS_OBJECTS_FILE,GU_LIST_ALL_BUCKETS_OBJECTS);
					
			}
            if(i == 0 || j== 0)
			{
				printf("get user success\n");
				snprintf(buf,MAX_LENGTH,"OK");
			}
			else
			{
				printf("get user failed\n");
				snprintf(buf,MAX_LENGTH,"Error");
			}
		}
		write(connfd,buf,strlen(buf));

	}
	else if(strstr(buf, "PUTBUCKET") ==buf)
	{
		j = split(&command,buf);
		for(k=0;k<j;k++)
			printf("command: %s\t",command.argv[i]);
		if(j == 3)
		{
			
			i = put_bucket(command.argv[1],command.argv[2]);
			if(i == 0)
			{
				printf("create bucket ok");
				snprintf(buf,sizeof(buf),"OK");
			}
			else
			{
				printf("create bucket failed!\n");
				snprintf(buf,sizeof(buf),"Error");

			}
			write(connfd,buf,strlen(buf));
		}
	}

	else if(strstr(buf, "PUTOBJECT") == buf)
	{
		j = split(&command,buf);
		for(k=0;k<j;++k)
			printf("command:%s\t",command.argv[k]);
		if(j ==4)
		{
			i = put_object(command.argv[1],command.argv[2],command.argv[3]);
			if(i == 0)
			{
				printf("put object ok\n");
				snprintf(buf,sizeof(buf),"OK");
			}
			else
			{
				printf("put object failed\n");
				snprintf(buf,sizeof(buf),"Error");
			}
		}
			write(connfd,buf,strlen(buf));

	}

	pthread_detach(pthread_self());
	printf("just for testing thread! end!!\n");
	close(connfd);
	return NULL;
}
int main()
{
	int listenfd,*connfd;
	socklen_t len;
	struct sockaddr_un servaddr,cliaddr;
	char buf[MAX_LENGTH];
	size_t servLen;
    //初始化命名空间树
	init_name_space();
	
	put_user("u5");
	put_bucket("b1","u5");
	put_bucket("b2","u5");
    
	pthread_t tid;

	if((listenfd = socket(PF_UNIX,SOCK_STREAM,0)) == -1)
	{
		printf("fail to creat socket!\n");
		return -1;
	}

	unlink(UNIXSTR_PATH);
	
	bzero(&servaddr,sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path,UNIXSTR_PATH);
    servLen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	if((bind(listenfd,(struct sockaddr *)&servaddr,servLen)) < 0)
	{
		printf("fail to bind!\n");
		close(listenfd);
		return -1;
	}
	if((listen(listenfd,20)) == -1)
	{
		printf("fail to listen!\n");
		return -1;
	}

	//主循环
	for(;;)
	{
		len = sizeof(cliaddr);
		connfd =(int*)malloc(sizeof(int));
		*connfd =accept(listenfd,(struct sockaddr *)&cliaddr,&len);

		if(*connfd == -1)
		{
			printf("fail to accept!\n");
			return -1;
		}
		//利用多线程处理每个连接请求
		pthread_create(&tid,NULL,&handle,connfd);
	}
	return 0;
}
