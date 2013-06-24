#ifndef _INPUT_H_
#define _INPUT_H_
//用于删除cline字符串开始的空格或制表符或换行
extern int del_blank(int pos, char *cline);

//获取cline字符串从pos开始的第一个字符串
extern int get_arg(char *arg, int pos, char *cline);

//s以空格为分割，解析cline字符串，存放到command中
extern int split(struct command_line * command, char cline[]);

#endif

