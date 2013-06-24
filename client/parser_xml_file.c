/*
*  gcc -I/usr/local/include/libxml2 -L/usr/local/lib -lxml2 -lz -lm -o parser parser_xml_file.c
*/
#include "common.h"
#include "parser_xml_file.h"

int doObjectExistsInBucket(char *ObjectName,char *BucketName)
{
    xmlDocPtr doc; // 定义解析文档指针
    xmlNodePtr curNode,bucNode,objNode;//定义节点指针（在各个节点之间移动）
    xmlChar *szKey;//定义临时字符串变量

    int flag =0;
    char *szDocName=XML_FILE_NAME;
    //szDocName = (char *)xmlFileName;
    doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_RECOVER);
    if (doc == NULL)
    {
        printf("document not parsed successfully");
        return -1;
    }

    //确定文档根元素
    curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL)
    {
        printf("empty document");
        xmlFreeDoc(doc);
        return -1;
    }
    curNode = curNode->xmlChildrenNode;
    bucNode = curNode;

    //printf("--------------------------------\n");
    while (bucNode != NULL) //打印第一层节点
    {
        if ((xmlStrcmp(bucNode->name,(xmlChar *)BucketName))==0)
        {

            objNode = bucNode->xmlChildrenNode;
            while (objNode!=NULL)//打印第二层节点
            {
                //printf("%s\n",objNode->name);
                if ((xmlStrcmp(objNode->name,(xmlChar *)ObjectName))==0)
                {
                    flag = 1;
                    return 0;
                }
                objNode=objNode->next;
            }
        }

        bucNode = bucNode->next;
    }
    xmlFreeDoc(doc);
    if (flag == 0)
        return -1;
}

int doBucketExists(char *BucketName)
{

    xmlDocPtr doc; // 定义解析文档指针
    xmlNodePtr curNode,bucNode,objNode;//定义节点指针（在各个节点之间移动）
    xmlChar *szKey;//定义临时字符串变量

    char *xmlFileName = XML_FILE_NAME;
    int flag =0;
    char *szDocName;
    szDocName = (char *)xmlFileName;
    doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_RECOVER);
    if (doc == NULL)
    {
        printf("document not parsed successfully");
        return -1;
    }

    //确定文档根元素
    curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL)
    {
        printf("empty document");
        xmlFreeDoc(doc);
        return -1;
    }
    curNode = curNode->xmlChildrenNode;
    bucNode = curNode;

    // printf("--------------------------------\n");
    while (bucNode != NULL) //打印第一层节点
    {
        if ((xmlStrcmp(bucNode->name,(xmlChar *)BucketName))==0)
        {
            flag =1;
            break;
        }
        bucNode = bucNode->next;
    }
    xmlFreeDoc(doc);

    if (flag == 0)
    {
        //printf("buckets not exists!\n");
        return -1;
    }
    else return 0;
}

//从缓存文件中读取对应fileName的Hash值
int getHashAndIPFromCacheFile(char *cacheFileName,char *fileName,char hash[],char ip[])
{
    printf("cacheFileName:%s\n",cacheFileName);
    printf("fileName:%s\n",fileName);

    struct command_line command;
    char cline[COMMAND_LINE];
    FILE*fp;
    int flag=0;
    // char buf[MAX_LINE];
    if ((fp=fopen(cacheFileName,"r"))==NULL)
    {
        printf("cannot open cacheFile\n");
        return -1;
    }
    while (fgets(cline, COMMAND_LINE, fp) != NULL)
    {
        // printf("%s",cline);
        if (split(&command, cline) == -1)
            return -1;
        printf("%s\t%s\t%s\n",command.argv[0],command.argv[1],command.argv[2]);

        if (strcasecmp(command.name, fileName) == 0)
        {
            flag=1;
            char *p1 = strchr(cline,'\t');
            char *p2 = rindex(cline,'\t');
            int k1 = p1-cline;
            int k2 = p2-cline;
            k1++;

            //char *temphash = (char *)malloc(p2-p1);

            int ip_len=strlen(cline)-k2-2;
            printf("ip_len:%d\n",ip_len);
            //char *tempip = (char *)malloc(ip_len);

            strncpy(hash,&cline[k1],p2-p1-1);
            hash[p2-p1-1]='\0';

            strncpy(ip,&cline[k2+1],ip_len);
            ip[ip_len]='\0';

            // printf("hash:%s\n",hash);
            //printf("ip:%s\n",ip);
            //return 0;
            break;
        }
    }
    if (flag ==0)
    {
        printf("cannot find the file in cacheFile\n");
        return -1;
    }
    return 0;
}

//缓存文件
int parserToCacheFile(char *cacheFileName,char *xmlFileName)
{
    FILE *fp;
    if ((fp=fopen(cacheFileName,"w"))==NULL)
    {
        printf("cannot open file\n");
        exit(-1);
    }
    else
    {
        printf("create the cacheFile succeed!\n");
    }
    xmlDocPtr doc; //定义解析文档指针
    xmlNodePtr rootNode,bucNode,objNode,properNode;//定义节点指针（在各个节点之间移动）

    char *szDocName = (char *)xmlFileName;
    doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_RECOVER);
    if (doc == NULL)
    {
        printf("document not parsed successfully");
        return -1;
    }

    //确定文档根元素
    rootNode = xmlDocGetRootElement(doc);
    if (rootNode == NULL)
    {
        printf("empty document");
        xmlFreeDoc(doc);
        return -1;
    }
    bucNode = rootNode->xmlChildrenNode;
    //bucNode = rootNode->children;
    while (bucNode != NULL) //第一层节点
    {
        objNode = bucNode->xmlChildrenNode;
        char hash_temp[33];
        char ip_temp[20];
        while (objNode!=NULL)//第二层节点
        {
            bzero(&hash_temp,sizeof(hash_temp));
            bzero(&ip_temp,sizeof(ip_temp));
            properNode = objNode->xmlChildrenNode;
            while (properNode !=NULL) //第三层节点
            {
                if (!xmlStrcmp(properNode->name, BAD_CAST "hash"))
                    strcpy(hash_temp,xmlNodeGetContent(properNode));
                if (!xmlStrcmp(properNode->name, BAD_CAST "ip"))
                    strcpy(ip_temp,xmlNodeGetContent(properNode));
                properNode = properNode->next;
            }
            fprintf(fp,"/%s/%s/%s\t%s\t%s\n",rootNode->name,bucNode->name,objNode->name,hash_temp,ip_temp);
            objNode=objNode->next;
        }

        bucNode = bucNode->next;
    }
    xmlFreeDoc(doc);
    fclose(fp);
    return 0;
}

//list objects in bucket
int listObjectsInBucket(char *xmlFileName,char *BucketName)
{

    xmlDocPtr doc; // 定义解析文档指针
    xmlNodePtr curNode,bucNode,objNode;//定义节点指针（在各个节点之间移动）
    xmlChar *szKey;//定义临时字符串变量

    int flag =0;
    char *szDocName;
    szDocName = (char *)xmlFileName;
    doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_RECOVER);
    if (doc == NULL)
    {
        printf("document not parsed successfully");
        return -1;
    }

    //确定文档根元素
    curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL)
    {
        printf("empty document");
        xmlFreeDoc(doc);
        return -1;
    }
    curNode = curNode->xmlChildrenNode;
    bucNode = curNode;

    printf("--------------------------------\n");
    while (bucNode != NULL) //打印第一层节点
    {
        if ((xmlStrcmp(bucNode->name,(xmlChar *)BucketName))==0)
        {
            flag =1;
            printf("objects in %s:\n",BucketName);
            objNode = bucNode->xmlChildrenNode;
            while (objNode!=NULL)//打印第二层节点
            {
                printf("%s\n",objNode->name);
                objNode=objNode->next;
            }
            break;
        }

        bucNode = bucNode->next;
    }
    if (flag == 0)
    {
        printf("buckets not exists!\n");
    }
    xmlFreeDoc(doc);
    return 0;

}

//list buckets
int listBuckets(char *xmlFileName)
{
    xmlDocPtr doc; // djf定义解析文档指针
    xmlNodePtr curNode,bucNode;//定义节点指针（在各个节点之间移动）
    xmlChar *szKey;//定义临时字符串变量
    int flag = 0;
    char *szDocName;
    szDocName = (char *)xmlFileName;

    doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_NOBLANKS); //以忽略空格方式打开
    if (doc == NULL)
    {
        printf("document not parsed successfully");
        return -1;
    }

    //确定文档根元素
    curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL)
    {
        printf("empty document");
        xmlFreeDoc(doc);
        return -1;
    }
    curNode = curNode->xmlChildrenNode;
    bucNode = curNode;
    if (bucNode != NULL)
    {
        flag = 1;
        printf("-----------------------------------------\n");
        printf("buckets: \n");
        while (bucNode !=NULL)
        {
            printf("%s\n",(char *)bucNode->name);
            bucNode = bucNode->next;
        }
    }
    else
    {
            printf("no buckets exists!\n");
    }
    xmlFreeDoc(doc);
    return 0;
}

//list buckets and objects
int listAll(char *xmlFileName)
{
    xmlDocPtr doc; // djf定义解析文档指针
    xmlNodePtr curNode,bucNode,objNode;//定义节点指针（在各个节点之间移动）
    xmlChar *szKey;//定义临时字符串变量
    int flag=0;
    char *szDocName;
    szDocName = (char *)xmlFileName;
    doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_NOBLANKS);
    if (doc == NULL)
    {
        printf("document not parsed successfully");
        return -1;
    }

    //确定文档根元素
    curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL)
    {
        printf("empty document");
        xmlFreeDoc(doc);
        return -1;
    }

    //curNode=curNode->xmlChildrenNode;
    bucNode = curNode->xmlChildrenNode;
    printf("------------------------------------------------\n");

    while (bucNode != NULL) //打印第一层节点
    {
        flag = 1;
        printf("-------------------------\n");
        printf("bucket:%s\n",bucNode->name);
        objNode = bucNode->xmlChildrenNode;
        printf("objects:\n");
        while (objNode!=NULL)//打印第二层节点
        {
            printf("%s\t",objNode->name);
            objNode=objNode->next;
        }
        printf("\n");
        bucNode = bucNode->next;
    }
    if (flag == 0)
    {
        printf("no buckets exists!\n");
    }
    xmlFreeDoc(doc);
    return 0;
}

//int main()
//{
//  char *bucketName = "b3";
//  //parserToCacheFile(CACHE_FILE_NAME,XML_FILE_NAME);
//  //listObjectsInBucket(XML_FILE_NAME,bucketName);
//  //listBuckets(XML_FILE_NAME);
//  listAll(XML_FILE_NAME);
//  return 0;
//}
