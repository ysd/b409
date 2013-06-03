#include"global.h"
#include<libxml/parser.h>
#include<libxml/tree.h>
#define FILE_PATH_LEN	1024
#define XML_STDIN_OUT_FILE  "-"
#define XML_BUFSZ 1024
#define XML_FILE	"zz.xml"
/***** xml_sock_msg ******/
#define XML_SOCK_MSG_ROOT   "sock_msg"
#define XML_SOCK_MSG_TYPE   "type"
#define XML_SOCK_MSG_DST_IP "dest_ip"
#define XML_SOCK_MSG_FN     "file_name"
#define SOCKMSG_TYPE_WRITE						'M'
#define SOCKMSG_TYPE_REMOVE						'N'
#define SOCKMSG_TYPE_OPEN_WITH_OTRUNC			'O'
#define SOCKMSG_TYPE_CREAT_ON_EXIST				'P'
/* msg parser flag */
#define SOCKMSG_TYPE_PARSE_OK   (1 << 0)
#define SOCKMSG_DSTIP_PARSE_OK  (1 << 1)
#define SOCKMSG_FN_PARSE_OK     (1 << 2)
#define SOCKMSG_PARSE_OK        (SOCKMSG_TYPE_PARSE_OK | SOCKMSG_DSTIP_PARSE_OK | SOCKMSG_FN_PARSE_OK)
/* sock_msg definition */
typedef struct{
	u8 type; //MSG_TYPE_UPDATE
	u8 dest_ip[INET_ADDRSTRLEN];
	u8 file_name[FILE_PATH_LEN];
}SOCK_MSG;
#define SOCK_MSG_TYPE   sock_msg->type
#define SOCK_MSG_FN     sock_msg->file_name
#define SOCK_MSG_DST_IP sock_msg->dest_ip
u32 atoi_u32(char *p)
{
	u32 i = 0;
	u32 j;
	char * ch = p;
	while(*ch != 00){
		if(*ch < '0' || *ch > '9'){
			fprintf(stderr,"Invalid character found in u32 string!\n");
			return -1;
		}
		j = (u32)(*ch - '0');
		i = 10*i + j;
		ch++;
	}
	return i;
}
u64 atoi_u64(char *p)
{
	u64 i = 0;
	u64 j;
	char * ch = p;
	while(*ch != 00){
		if(*ch < '0' || *ch > '9'){
			fprintf(stderr,"Invalid character found in u64 string!\n");
			return -1;
		}
		j = (u64)(*ch - '0');
		i = 10*i + j;
		ch++;
	}
	return i;
}
inline void u8_to_str(char ch,char *buf,u32 bufsiz)
{
	bzero(buf,bufsiz);
	snprintf(buf,bufsiz,"%c",ch);
}
inline void u32_to_str(u32 i,char *buf,u32 bufsiz)
{
	bzero(buf,bufsiz);
	snprintf(buf,bufsiz,"%u",i);
}
inline void u64_to_str(u64 i,char *buf,u32 bufsiz)
{
	bzero(buf,bufsiz);
	snprintf(buf,bufsiz,"%llu",i);
}
u32 sock_msg_to_xml(SOCK_MSG *sock_msg,u8 *xml_file)
{
	u8 *std_io_file = XML_STDIN_OUT_FILE;
	u8 *dest_file = (xml_file == NULL)?std_io_file : xml_file;
	u8 buf[XML_BUFSZ];
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL,BAD_CAST XML_SOCK_MSG_ROOT);
	xmlDocSetRootElement(doc,root_node);
	u8_to_str(SOCK_MSG_TYPE,buf,XML_BUFSZ);
	xmlNewChild(root_node,NULL,BAD_CAST XML_SOCK_MSG_TYPE,BAD_CAST buf);
	xmlNewChild(root_node,NULL,BAD_CAST XML_SOCK_MSG_DST_IP,BAD_CAST SOCK_MSG_DST_IP);
	xmlNewChild(root_node,NULL,BAD_CAST XML_SOCK_MSG_FN,BAD_CAST SOCK_MSG_FN);
	xmlSaveFormatFileEnc(dest_file,doc,"UTF-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}
u32 xml_to_sock_msg(u8 *xml_file,SOCK_MSG *sock_msg)
{
	u8 *std_io_file = XML_STDIN_OUT_FILE;
	u8 *src_file = (xml_file == NULL)?std_io_file : xml_file;
	u32 ret = 0;
	u8 parse_ok = 00;
	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlChar *szKey;
	if((doc = xmlReadFile(src_file,"UTF-8",XML_PARSE_RECOVER)) == NULL){
		fprintf(stderr,"DOCUMENT PARSE FAIL : %s\n",src_file);
		ret = 1;
		goto op_over;
	}
	if((curNode = xmlDocGetRootElement(doc)) == NULL){
		fprintf(stderr,"EMPTY DOCUMENT!\n");
		ret = 2;
		goto op_over;
	}
	if(xmlStrcmp(curNode->name,BAD_CAST XML_SOCK_MSG_ROOT) != 0){
		fprintf(stderr,"NOT A RPL_MSG!\n");
		ret = 3;
		goto op_over;
	}
	curNode = curNode->children;
	while(curNode){
		if(!(parse_ok & SOCKMSG_TYPE_PARSE_OK) && xmlStrcmp(curNode->name,(const xmlChar *)XML_SOCK_MSG_TYPE) == 0){
			szKey = xmlNodeGetContent(curNode);
			SOCK_MSG_TYPE = *(u8 *)szKey;
			xmlFree(szKey);
			parse_ok |= SOCKMSG_TYPE_PARSE_OK;
			if(parse_ok == SOCKMSG_PARSE_OK)
				break;
		}else if(!(parse_ok & SOCKMSG_DSTIP_PARSE_OK) && xmlStrcmp(curNode->name,(const xmlChar *)XML_SOCK_MSG_DST_IP) == 0){
			szKey = xmlNodeGetContent(curNode);
			strcpy(SOCK_MSG_DST_IP,szKey);
			xmlFree(szKey);
			parse_ok |= SOCKMSG_DSTIP_PARSE_OK;
			if(parse_ok == SOCKMSG_PARSE_OK)
				break;
		}else if(!(parse_ok & SOCKMSG_FN_PARSE_OK) && xmlStrcmp(curNode->name,(const xmlChar *)XML_SOCK_MSG_FN) == 0){
			szKey = xmlNodeGetContent(curNode);
			strcpy(SOCK_MSG_FN,szKey);
			xmlFree(szKey);
			parse_ok |= SOCKMSG_FN_PARSE_OK;
			if(parse_ok == SOCKMSG_PARSE_OK)
				break;
		}
		curNode = curNode->next;
	}
op_over:
	xmlFreeDoc(doc);
	if(parse_ok != SOCKMSG_PARSE_OK){
		ret = 4;
		fprintf(stderr,"SOCK_MSG PARSE FAIL!\n");
	}
	return ret;
}
int main()
{
	SOCK_MSG sockmsg;
	SOCK_MSG *sock_msg = &sockmsg;
	SOCK_MSG_TYPE = SOCKMSG_TYPE_WRITE;
	strcpy(SOCK_MSG_FN,"testfile");
	strcpy(SOCK_MSG_DST_IP,"127.0.0.1");
	sock_msg_to_xml(sock_msg,XML_FILE);
	bzero(sock_msg,sizeof(sockmsg));
	xml_to_sock_msg(XML_FILE,sock_msg);
	printf("sock_msg.type -- %c\n",SOCK_MSG_TYPE);
	printf("sock_msg.fn   -- %s\n",SOCK_MSG_FN);
	return 0;
}
