/*
*用户认证模块所用到的
*compiler command:
*gcc -L/usr/local/lib -ltokyocabinet -ltokyotyrant -o authen authen.c
*/

#include "common.h"
#include "authen.h"
//#include <tcrdb.h>
bool PutValue(char *key,char *value)
{
    TCRDB*rdb;
    rdb = tcrdbnew();
    if (!tcrdbopen(rdb,TTIP,TTPORT))
    {
        printf("PutValue :cannot open \n");
        exit(-1);
    }
    if (!tcrdbput2(rdb,key,value))
    {
        printf("cannot put\n");
        return false;
    }
    else
    {
        printf("succeed put\n");
        return true;
    }
    tcrdbclose(rdb);
}

char *GetValue(char *key)
{
    TCRDB*rdb;
    rdb = tcrdbnew();
    if (!tcrdbopen(rdb,TTIP,TTPORT))
    {
        printf("GetValue:cannot open \n");
        exit(-1);
    }
    char *value = (char *)malloc(50);
     value = tcrdbget2(rdb,key);
    if (value)
    {
        printf("the value of key %s is:%s\n",key,value);
        return value;
    }
    else
    {
        printf("key %s doesnot exists\n",key);
        return NULL;
    }
    tcrdbclose(rdb);
}
int init_tt()
{
    //在数据库中添加1000个用户
    //用户名为u1-1000
    //密码为u1-1000
    int i=0;
    char user[]="u";
    char temp[10];
    //strcpy(temp,user);
    for (i=1;i<=1000;++i)
    {
        bzero(&temp,sizeof(temp));
        snprintf(temp,sizeof(temp),"%s%d",user,i);
        printf("%s\n",temp);
        PutValue(temp,temp);
    }
    return 0;
}

//int main(int argc,char *argv[])
//{
//   // init_tt();
//
//    char *value = GetValue("u1");
//    printf("the value:%s\n",value);
//    free(value);
//
//    return 0;
//}
