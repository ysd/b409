#ifndef _COMMAND_H_
#define _COMMAND_H_
extern int init(struct sockaddr_in *sin, int *lfd, int sock_opt);
extern int do_put(int cfd, char *file);
extern int do_get(int cfd, char *file);
extern int do_getuser(char *name);
extern int do_createxml(int sock_fd,char *userName);
extern int do_login(int cfd,char *name,char *password);
extern int do_putxml(int cfd,char *name);
#endif
