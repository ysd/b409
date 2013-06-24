#include "common.h"
#include "global.h"
#include "md_type.h"
#include "md5.h"
#include "utility.h"
#include "list_head.h"
#include "name_buf.h"
#include "nss.h"
#include "xml.h"
#include "command.h"
#include "authen.h"

int do_createxml(int sock_fd,char *userName)
{
	int n;
	char buf[MAX_LENGTH];
	/*****************FIRST WRITE****************/
	snprintf(buf,sizeof(buf),"%s /%s","GETUSER",userName);
	write(sock_fd,buf,strlen(buf));

	/****************FIRST READ******************/
	n = read(sock_fd,buf,MAX_LENGTH);
	buf[n]='\0';
	if(n > 0)
	{
		printf("receive from main_server: %s\n",buf);
		if(buf[0] == 'O')
			return 0;
	}
	else
	{
		printf("cannot receive from main_server\n");
		return -1;
	}
}
	
int init(struct sockaddr_in *sin, int *lfd, int sock_opt)
{
    int tfd;

    bzero(sin, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = INADDR_ANY;
    sin->sin_port = htons(PORT);

    if ( (tfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("fail to creat socket");
        return -1;
    }

    //socketclose之后不会立即关闭，而经历time_wait，为了在这个时间段内重用，则采用下面设置
    setsockopt(tfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));

    if ( (bind(tfd, (struct sockaddr *)sin, sizeof(struct sockaddr_in))) == -1)
    {
        perror("fail to bind");
        return -1;
    }

    if ( (listen(tfd, 20)) == -1)
    {
        perror("fail to listen");
        return -1;
    }

    *lfd = tfd;

    return 0;
}


int do_login(int cfd,char *name,char *password)
{
    char buf[MAX_LENGTH];
    int flag = 0;
    /*****************第一次向客户端发送内容******************************/
    char *password1 = GetValue(name);
    if (strcmp(password1,password)==0)
    {

        printf("login succeed\n");
		flag=1;
        bzero(&buf,sizeof(buf));
        sprintf(buf, "%d\n", flag);
        write(cfd, buf, strlen(buf)); //服务器向客户端发送“1“命令
    }
    else
    {
        bzero(&buf,sizeof(buf));
        sprintf(buf, "%d\n", flag);
        write(cfd, buf, strlen(buf)); //服务器向客户端发送“0“命令
        return -1;
    }
}
int do_getuser(char *name)
{
	int i;
	i=get_user(name,ALL_BUCKETS_OBJECTS_FILE,GU_LIST_ALL_BUCKETS_OBJECTS);
	if(i == 0)
	 {
		 printf("create directory tree succeed!\n");
		 return 0;
	 }
	 else if(i == 1)
	 {
		int j;
		put_user(name);
		j=get_user(name,ALL_BUCKETS_OBJECTS_FILE,GU_LIST_ALL_BUCKETS_OBJECTS);
		printf("j=%d\n",j);
		if(j == 0)
			return 0;
		else return -1;
		 // printf("user not exists\n");
	 }
	 else
	 {
		 printf(" do_getuser() error\n");
		 return -1;
	 }
}
	 

int do_putxml(int cfd,char * name)
{
	FILE *fp;
    struct stat statbuf;
	int i, n, fd;
    char buf[1024];

	//文件不存在
    if(access(ALL_BUCKETS_OBJECTS_FILE,F_OK) < 0)
	{
		//如果文件不存在，则发送"NO"
		printf("the xml file not exists!\n");
		write(cfd,"NO",strlen("NO"));
		return 0;
	}
	if ((fd = open(ALL_BUCKETS_OBJECTS_FILE,O_RDONLY)) == -1)
    {
		printf("cannot open the xml file\n");
        return -1;
    }
    fstat(fd, &statbuf);
    if (!S_ISREG(statbuf.st_mode))
    {
       
		printf("xml file is not a normal file\n");
        return -1;
    }
    /*****************文件传输前第一次写******************************/
    sprintf(buf, "OK %ld", statbuf.st_size);
	write(cfd, buf, strlen(buf)); //服务器向客户端发送“OK 字节数“命令

   /****************文件传输前第一次读取客户端内容**************************/
   n = read(cfd, buf, MAX_LENGTH);//接收客户端发送的“RDY“字符串，之后双方进行数据传输
   buf[n]='\0';
   printf("%s\n",buf);

  /********************文件传输***************************************/
  while (1)
  {
      n = read(fd, buf, MAX_LENGTH);
      if (n > 0)
          write(cfd, buf, n);
      else if (n == 0)
      {
          printf("file transfer OK\n");
          break;
      }
      else
      {
          printf("fail to read\n");
          return -1;
      }
  }
  close(fd);
  return 0;
}
int do_put(int cfd, char *file)//接收到客户端请求后将文件传输
{
    struct stat statbuf;
    int n, fd;
    int res = -1;
    char buf[1024];

    if ( (fd = open(file, O_RDONLY)) == -1)
    {
        write(cfd, "ERR open server file\n", strlen("ERR open server file\n"));
        return -1;
    }

    fstat(fd, &statbuf);

    if (!S_ISREG(statbuf.st_mode))
    {
        write(cfd, "ERR: server file should be a regular\n",
              strlen("ERR: server file should be a regular\n"));
        close(fd);
        return -1;
    }
    /*****************第一次向客户端发送内容******************************/
    sprintf(buf, "OK %ld\n", statbuf.st_size);
    write(cfd, buf, strlen(buf)); //服务器向客户端发送“OK 字节数“命令

    /****************接收客户端的第二次写的内容**************************/
    n = read(cfd, buf, MAX_LENGTH);//接收客户端发送的“RDY“字符串，之后双方进行数据传输
    buf[n]='\0';
    //printf("%s\n",buf);

    /********************文件传输***************************************/
    while (1)
    {
        n = read(fd, buf, MAX_LENGTH);
        if (n > 0)
            write(cfd, buf, n);
        else if (n == 0)
        {
            printf("file transfer OK\n");
            break;
        }
        else
        {
            perror("fail to read");
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}


int do_get(int cfd, char *file)
{
    struct stat statbuf;
    int n, fd;
    char buf[1024];
    int len;

    if ( (fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
    {
        printf("cannot open the file\n");
        write(cfd, "ERR open server file\n", strlen("ERR open server file\n"));
        close(fd);
        return -1;

    }

    if ( (fstat(fd, &statbuf)) == -1)
    {
        printf("cannot get the stat of file\n");
        write(cfd, "ERR stat server file\n", strlen("ERR stat server file\n"));
        close(fd);
        return -1;
    }

    if (!S_ISREG(statbuf.st_mode))
    {
        write(cfd, "ERR server path should be a regular file\n",
              strlen("ERR server path should be a regular file\n"));
        close(fd);
        return -1;
    }
    /***************服务器端第二次读**************************/
    n = read(cfd,buf,MAX_LENGTH);
    buf[n]='\0';
    len = atoi(&buf[5]);
//printf("%s size:%d\n",buf,len);

    /****************服务器第一次写*********************************/
    write(cfd, "server is OK", strlen("server is OK"));

    /******************文件传输*********************************/
//确定接收文件的大小,大小由之前双方协商得出，保证写入固定大小的字节
    char buf1[1000];
	while ((n = read(cfd,buf1,MAX_LENGTH))>0)
    {
        //printf("%s\n",buf1);
        if (write(fd,buf1,n)!=n)
        {
            printf("write error\n");
            return -1;
        }
        len-=n;
        if (len ==0)
        {
            fsync(fd);
            printf("file transfer OK\n");
            break;
        }
    }
    if (n<0)
    {
        printf("read error\n");
        return -1;
    }
    close(fd);
    return 0;
}
