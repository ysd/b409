#include "common.h"
#include "command.h"
#include "nss_msg.h"
int server_start(void)
{
    struct sockaddr_in sin;
    struct sockaddr_in cin;
	struct command_line command;
    int lfd;//监听套接字
    int cfd;//连接套接字
	char buf[MAX_LENGTH];
    char str[ADDR_LEN];
    int sock_opt = 1;
    int i,j,n,fd,m=0;
    pid_t pid;
    socklen_t len;
	len = sizeof(struct sockaddr_in);
	//初始化
    if (init(&sin, &lfd, sock_opt) == -1)
        exit(1);
    printf("waiting connections ...\n");

    while (1)
    {
		//接收client连接
        if ( (cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1)
        {
            printf("fail to accept\n");
            return -1;
        }

        if((pid = fork()) < 0)
        {
            printf("fail to fork\n");
            return -1;
        }
        else if (pid == 0) //子进程
        {
            close(lfd);//关闭监听套接字
            while (1)
            {
                /****************s接收客户端第一次写的内容************/
                m = read(cfd, buf, MAX_LENGTH);
                buf[m]='\0';
                if (m<0)
					return -1;
                printf("receive from client %s\n",buf);

                if (strstr(buf, "GET") == buf) //以GET开头的命令
                {
                    if (do_put(cfd, &buf[4]) == -1)
                        printf("error occours while putting\n");
                }
                else if (strstr(buf, "PUT") == buf)
                {
                    char *position = strchr(buf,'/');
                    int k=position-buf;
                    if (do_get(cfd,&buf[k])==-1)
                        printf("error occours while getting\n");
                }
                //对于客户端登录信息的处理
                else if (strstr(buf,"LOGIN") ==buf)
                {
					i = split(&command,buf);
					if(i ==3)
					{
						if (do_login(cfd,command.argv[1],command.argv[2]) ==-1)
							 printf("%s login failed\n",command.argv[1]);
					}
					for(j=0;j<i;j++)
						free(command.argv[j]);
                }
				//接收客户端请求xml文件
				else if (strstr(buf,"XMLGET") == buf)
				{
					i = split(&command,buf);
					if(i == 2)
					{
						connect_to_nss_server(&fd);
						// 与主进程交互
						if(get_user_msg_client(fd,command.argv[1]) == 0)
						{
							printf("server:create user xml succeed\n");
							if(do_putxml(cfd,command.argv[1]) == -1)
								printf("put the xml file failed\n");
						}		
						else
							printf("login_server:create user xml failed!\n");
					}
					for(j=0;j<i;j++)
						free(command.argv[j]);

				}
				//接收断开连接请求
                else if (strstr(buf, "BYE") == buf)
                    break;
                else
                {
                    printf("wrong command\n");
                    exit(0);
				}
            }
            close(cfd);
            exit(0);
        }
        else
            close(cfd);
    }
    return 0;
}
int main()
{
	server_start();
	return 0;
}
