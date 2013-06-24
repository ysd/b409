/* command_split.c
*  功能：实现一个命令行分割
*  6.21
*/

#include "common.h"

//用于删除cline字符串开始的空格或制表符或换行
int del_blank(int pos, char *cline)
{
    while (cline[pos] != '\0' && (cline[pos] == ' ' || cline[pos] == '\t' || cline[pos]=='\n'))
        pos++;
    return pos;
}
//获取cline字符串从pos开始的第一个字符串
int get_arg(char *arg, int pos, char *cline)
{
    int i = 0;
    while (cline[pos] != '\0' && cline[pos] != ' ' && cline[pos] != '\t' && cline[pos]!='\n')
    {
        arg[i++] = cline[pos++];
    }
    arg[i]='\0';
    return pos;
}

//s以空格为分割，解析cline命令行，存放到command结构体中
int split(struct command_line * command, char cline[])
{
    int i;
    int pos = 0;

    pos = del_blank(pos, cline);

    i = 0;
    while (cline[pos] != '\0')
    {
        if ((command->argv[i] = (char *)malloc(MAX_LENGTH)) == NULL)
        {
            perror("fail to malloc");
            return -1;
        }

        pos = get_arg(command->argv[i], pos, cline);
        //printf("argv[%d]:%s\n",i,command->argv[i]);

        i++;
        pos = del_blank(pos, cline);
    }

    command->argv[i] = NULL;
    command->name = command->argv[0];

    return i;
}
