#include"global.h"
#include"md_type.h"
#include"md5.h"
#include"utility.h"
#include"list_head.h"
#include"name_buf.h"
#include"nss.h"
#include"xml.h"
#define U1	"u1"
#define U2	"u2"
#define U3	"u3"
#define U4	"u4"
#define U5	"u5"
#define U6	"u6"
#define U7	"u7"
#define U8	"u8"
#define U9	"u9"
#define U10	"u10"
#define U11	"u11"
#define U12	"u12"
#define B1	"b1"
#define B2	"b2"
#define B3	"b3"
#define B4	"b4"
#define B5	"b5"
#define B6	"b6"
#define B7	"b7"
#define B8	"b8"
#define O1	"o1"
#define O2	"o2"
#define O3	"o3"
#define O4	"o4"
#define O5	"o5"
#define O6	"o6"
#define O7	"o7"
#define O8	"o8"
#define O9	"o9"
#define O10	"o10"
#define O11	"o11"
#define O12	"o12"
#define O13	"o13"
static char fp[MAX_PATH];
static int init_md_of_obj(char *u,char *b,char *o)
{
	Meta_Data md;
	char md5c[MD5_CHECKSUM_SZ];
	char md5s[MD5_STRING_LEN];
	int path_len;
	time_t t = time(NULL);
	bzero(fp,MAX_PATH);
	snprintf(fp,MAX_PATH,ABS_PATH_FMT,u,b,o);
	path_len = strlen(fp);
	md5(fp,path_len,md5c);
	bzero(md5s,MD5_STRING_LEN);
	md5_2_str(md5c,md5s);
	bzero(&md,MD_SZ);
	md.atime = t;
	md.ctime = t;
	md.mtime = t;
	md.size = 1024;
	strcpy(md.replica[0].rep_ip,"192.168.0.243");
	if(md_put(md5s,&md) != 0){
		return 1;
	}
	return 0;
}
int main()
{
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	init_name_space();
	put_user(U1);
	put_user(U2);
	put_user(U3);
	put_user(U4);
	put_user(U5);
	put_user(U6);
	put_user(U7);
	put_user(U8);
	put_user(U9);
	put_user(U10);
	put_user(U11);
	put_user(U12);
	prt_ulist();
	prt_uhash();
	printf("----------------------\n");
	put_bucket(B1,U5);
	put_bucket(B2,U5);
	put_bucket(B3,U5);
	put_bucket(B4,U5);
	put_bucket(B5,U5);
	prt_bhash();
	printf("----------------------\n");
	if(put_object(O1,B3,U5) == 0){
		init_md_of_obj(U5,B3,O1);
		printf("put o1 ok\n");
	}
	if(put_object(O2,B3,U5) == 0){
		init_md_of_obj(U5,B3,O2);
		printf("put o2 ok\n");
	}
	if(put_object(O3,B3,U5) == 0){
		init_md_of_obj(U5,B3,O3);
		printf("put o3 ok\n");
	}
	if(put_object(O4,B3,U5) == 0){
		init_md_of_obj(U5,B3,O4);
		printf("put o4 ok\n");
	}
	if(put_object(O5,B3,U5) == 0){
		init_md_of_obj(U5,B3,O5);
		printf("put o5 ok\n");
	}
	if(put_object(O6,B3,U5) == 0){
		init_md_of_obj(U5,B3,O6);
		printf("put o6 ok\n");
	}
	if(put_object(O7,B3,U5) == 0){
		init_md_of_obj(U5,B3,O7);
		printf("put o7 ok\n");
	}
	prt_ohash();
	get_bucket(B3,U5,LIST_OBJECT_FILE);
	get_user(U5,LIST_BUCKET_FILE,GU_LIST_BUCKETS);
	get_user(U5,ALL_BUCKETS_OBJECTS_FILE,GU_LIST_ALL_BUCKETS_OBJECTS);
	return 0;
}
