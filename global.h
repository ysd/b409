#ifndef _GLB_H
#define _GLB_H
#define _PROTOTYPE(function,params)		extern function()
#define MAX_PATH 260
#define MAX_ION_PATH 260
#define CACHE_PATH		"/mnt/cache/"
#define DATA_PATH		"/mnt/data_center/"
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<tchdb.h>
#include<tcutil.h>
#include<stdbool.h>
#include<stdint.h>
#include<string.h>
#include<time.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
/* socket */
#define Q_OUT_SERV_PORT 9374
#define SERV_IP "127.0.0.1"
#define LISTENQ 128
typedef struct{
    char path[MAX_PATH];		/* I/O on which file */
	char ion_key[MAX_ION_PATH]; /* only for write */
    time_t m_atime;				/* io time */
    int io_type;				/* read or write */
    u64 offset;	
	u32 length;
}q_out_req;
#define QOUT_REQ_SZ		sizeof(q_out_req)
#endif

