#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/un.h>
#include <errno.h>

#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <tchdb.h>
#include <tcutil.h>
#include <stdint.h>
#include <stdbool.h>
#include <tcrdb.h>
#include <pthread.h>

#define UNIXSTR_PATH "/usr/unix"
#define MSGKEY 1024
#define MAX_LENGTH 1024
#define PORT 8000 //server port 
#define MAIN_SERVER_PORT 8004 //main_server port 
#define ADDR_LEN 17
#define NAME_LEN 256
#define MAX_ARG 10

#define USERDATABASE "user.tch"
#define TTIP "192.168.0.152" //用户信息数据库名所在服务器,在152机器上已经运行一个/ttserver/userInfo.tch数据库
#define TTPORT 1987//用户信息数据库监听端口号

struct command_line{
	char *name;
	char *argv[MAX_ARG];
};
#endif
