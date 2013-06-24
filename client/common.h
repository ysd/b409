#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <libxml/parser.h>
#include <curl/curl.h>


#define XML_FILE_NAME "/usr/local/a.xml"
#define CACHE_FILE_NAME "/usr/local/cache_file.txt"//缓存文件名
#define DST "/usr/local/a.xml"
#define SAVE_PATH "/usr/local/tmp/"

#define SERVERIP "192.168.0.244"
#define PORT 8000

#define DATASERVERIP "192.168.0.244"
#define DATASERVERPORT 8888

#define MAX_LINE 1024
#define COMMAND_LINE 256
#define MAX_ARG 10
#define MAX_LENGTH 64
#define NAME_LEN 256

struct command_line{
	char *name;
	char *argv[MAX_ARG];
};

#endif

