#include"global.h"
#include"utility.h"
#include"list_head.h"
#include"name_buf.h"
/*
#include"xml.h"
*/
#include"nss_api.h"
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
int main()
{
	user_dir_t * u;
	bucket_t * b;
	object_t * o;
	init_name_space();
	add_user("super_user");
	add_user(U1);
	add_user(U2);
	add_user(U3);
	add_user(U4);
	add_user(U5);
	add_user(U6);
	add_user(U7);
	add_user(U8);
	add_user(U9);
	add_user(U10);
	add_user(U11);
	add_user(U12);
	prt_ulist();
	prt_uhash();
	printf("----------------------\n");
//	printf("del 6\n");
//	if(get_user_dir_by_name(U6,(void**)&u,OP_WITH_LOCK) == 0){
//		del_user(u);
//	}
//	prt_ulist();
//	prt_uhash();
//	printf("del 10\n");
//	if(get_user_dir_by_name(U10,(void**)&u,OP_WITH_LOCK) == 0){
//		del_user(u);
//	}
//	prt_ulist();
//	prt_uhash();
	if(get_user_dir_by_name(U4,(void**)&u,OP_WITH_LOCK) == 0){
		add_bucket(B1,u);
		add_bucket(B2,u);
		add_bucket(B3,u);
		add_bucket(B4,u);
		add_bucket(B5,u);
		prt_blist(u);
		prt_bhash();
//		if(get_bucket_by_name(B2,u,(void**)&b,OP_WITH_LOCK) == 0){
//			printf("-- del b2 -- \n");
//			del_bucket(b);
//			prt_blist(u);
//			prt_bhash();
//		}
//		if(get_bucket_by_name(B4,u,(void**)&b,OP_WITH_LOCK) == 0){
//			printf("-- del b3 -- \n");
//			del_bucket(b);
//			prt_blist(u);
//			prt_bhash();
//		}
//		if(get_bucket_by_name(B1,u,(void**)&b,OP_WITH_LOCK) == 0){
//			printf("-- del b4 -- \n");
//			del_bucket(b);
//			prt_blist(u);
//			prt_bhash();
//		}
		if(get_bucket_by_name(B3,u,(void**)&b,OP_WITH_LOCK) == 0){
			add_object(O1,b);
			add_object(O2,b);
			add_object(O3,b);
			add_object(O4,b);
			add_object(O5,b);
			add_object(O6,b);
			add_object(O6,b);
			add_object(O7,b);
			prt_olist(b);
			prt_ohash();
//			if(get_object_by_name(O2,b,(void**)&o,OP_WITH_LOCK) == 0){
//				printf("del o2\n");
//				del_object(o);
//				prt_olist(b);
//				prt_ohash();
//			}
//			if(get_object_by_name(O3,b,(void**)&o,OP_WITH_LOCK) == 0){
//				printf("del o3\n");
//				del_object(o);
//				prt_olist(b);
//				prt_ohash();
//			}
//			if(get_object_by_name(O7,b,(void**)&o,OP_WITH_LOCK) == 0){
//				printf("del o7\n");
//				del_object(o);
//				prt_olist(b);
//				prt_ohash();
//			}
//			if(get_object_by_name(O4,b,(void**)&o,OP_WITH_LOCK) == 0){
//				printf("del o4\n");
//				del_object(o);
//				prt_olist(b);
//				prt_ohash();
//			}
//			printf("now del b3,but still objects in b3...let's see what will happen!\n");
//			if(del_bucket(b) != 0){
//				fprintf(stderr,"del b3 fail!\n");
//			}
//			prt_blist(u);
//			prt_bhash();
		}
	}
	del_user(u);
	prt_ulist();
	prt_uhash();
	return 0;
}
