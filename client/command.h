#ifndef _COMMAND_H_
#define _COMMAND_H_

extern int do_connect(char *ip,struct sockaddr_in *sin, int *sock_fd);//客户端与server的连接
extern int do_login(const char *userName,const char *password ,int sock_fd);//客户端登录

extern int do_listAll();//列出所有的buckets和objects
extern int do_listBuckets();//列出所有的buckets
extern int do_listObjects(char *bucketName);//列出某一个bucket下所有的objects


int do_getxml(const char *userName ,int sock_fd);//向服务器端请求目录树
int do_getobject(char *ObjectName,char *BucketName,char *UserName);//获取某个bucket下的某个object
int do_putobject(char *ObjectName,char *BucketName, char *UserName);//将一个object放到某个bucket中

//extern int do_get_byself(const char *dst,int sock_fd);
extern int do_get(const char *src, const char *dst, int sock_fd);//完成客户端下载文件操作，src是源文件，dst是目的路径,文件名保持不变
extern int do_put(const char *src, const char *dst, int sock_fd);//完成客户端上传文件操作，src是源文件，dst是目的路径,文件名保持不变
extern int do_bye(int sock_fd);//断开与服务器的连接
#endif
