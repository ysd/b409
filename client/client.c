/*
*gcc -I/usr/local/include/libxml2 -L/usr/local/lib -lxml2 -lz -lm -o client client.c command.c input.c parser_xml_file.c
*/
/* command.c
*  功能：客户端
*  6.21
*/
#include "common.h"
#include "command.h"
#include "command_split.h"
int client()
{
    char cline[COMMAND_LINE];
    struct command_line command;
    int sock_fd;
    int i,j;
    int flag = 0;
    char userName[MAX_LENGTH];
    struct sockaddr_in sin;
    //首先连接服务器
    if (do_connect(SERVERIP, &sin, &sock_fd) == -1)
    {
        printf("cannot connect to server!\n");
        return -1;
    }
    else
        printf("connect to server succeed!\n");

    //循环主程序供用户输入各种命令
    printf("input command$: ");
    while (fgets(cline, MAX_LINE, stdin) != NULL)
    {
        //分割命令行为单个字符串，并存放到command结构体中
        if ((i=split(&command, cline)) == -1)
        {
            printf("split the command failed!\n");
            return -1;
        }

        if (strcasecmp(command.name, "login") == 0)
        {
            if (i!=3 || do_login(command.argv[1],command.argv[2],sock_fd) == -1)
            {
                printf("login failed\n");
                flag = 0;
            }
            else
            {

                strcpy(userName,command.argv[1]);
                printf("login succeed， userName:%s\n",userName);
                flag = 1;
            }
        }
        else if (strcasecmp(command.name,"xmlget") ==0)
        {
            if (i == 1 && flag == 1)
            {
                    printf("userName:%s\n",userName);
                if (do_getxml(userName,sock_fd) ==-1)
                    printf("get the xml file failed\n");
            }
            else
                printf("please login first!\n");
        }

        else if (strcasecmp(command.name, "getobject") == 0)
        {
            if (flag == 1 )
            {
                printf("comman.1:%s\n",command.argv[1]);
                printf("comman.2:%s\n",command.argv[2]);
                if (do_getobject(command.argv[1],command.argv[2],&userName)==-1)
                    printf("get the object failed\n");
            }
            else
                printf("please login first!\n");
        }
        else if (strcasecmp(command.name, "putobject") == 0)
        {
            if (flag == 1)
            {
                if (do_putobject(command.argv[1],command.argv[2],userName)==-1)
                    printf("put the object failed!\n");
            }
            else
                printf("please login first!\n");

        }
        else if (strcasecmp(command.name, "listall") ==0)
        {
            if (flag==1)
            {
                if (do_listAll() ==-1)
                    printf("cannot list all of the buckets and objects\n");
            }
            else
                printf("please login first!\n");
        }
        else if (strcasecmp(command.name, "listbuckets") ==0)
        {
            if (flag==1)
            {
                if (do_listBuckets() ==-1)
                    printf("cannot list buckets\n");
            }
            else
                printf("please login first!\n");
        }
        else if (strcasecmp(command.name, "listobjects") ==0)
        {
            if (flag==1)
            {
                if (do_listObjects(command.argv[1]) ==-1)
                    printf("cannot list objects\n");
            }
            else
                printf("please login first!\n");
        }

        else if (strcasecmp(command.name, "get") == 0)
        {
            if (do_get(command.argv[1], command.argv[2], sock_fd) == -1)
                exit(1);

        }
        else if (strcasecmp(command.name, "put") == 0)
        {
            if (do_put(command.argv[1], command.argv[2], sock_fd) == -1)
                exit(1);
        }
        else if (strcasecmp(command.name, "bye") == 0)
        {
            if (do_bye(sock_fd) == -1)
                exit(1);
            break;
        }
        else
        {
            printf("wrong command\n");
        }

        printf("i:%d\n",i);
            //free malloc
        for (j=0;j<i;j++)
            free(command.argv[j]);
        printf("your command$: ");
    }
    if (close(sock_fd) == -1)
    {
        printf("fail to close!\n");
        return -1;
    }
}
int main(void)
{
    client();
    return 0;
}
