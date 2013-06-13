#ifndef _NSS_H
#define _NSS_H
#include"global.h"
#include"name_buf.h"
#define ROOT_DIR	"/"
#define SHARED_DIR	"shared"
/* /user_dir/bucket/object */
#define ABS_PATH_FMT	"/%s/%s/%s"
enum{
	UID = 0,
	GID,
	ACL,
	UGA
};
#define SU_UID	0
#define SU_GID	0
#define O_R	04
#define O_W	02
#define O_X	01
#define G_R	(O_R << 3)
#define G_W	(O_W << 3)
#define G_X	(O_X << 3)
#define U_R	(G_R << 3)
#define U_W	(G_W << 3)
#define U_X	(G_X << 3)
#define get_uid(p)			((p)->uga[UID])
#define get_gid(p)			((p)->uga[GID])
#define get_acl(p)			((p)->uga[ACL])
#define set_uid(p,v)		do{	\
	typeof((p)->uga[0]) _v = (v);	\
	(p)->uga[UID] = _v;	\
}while(0)
#define set_gid(p,v)		do{	\
	typeof((p)->uga[0]) _v = (v);	\
	(p)->uga[GID] = _v;	\
}while(0)
#define set_acl(p,v)		do{	\
	typeof((p)->uga[0]) _v = (v);	\
	(p)->uga[ACL] = _v;	\
}while(0)
typedef struct{
	pthread_mutex_t mutex;			/* protect the user list */
	struct list_head user_dirs;		/* head of user_dir list */
}root_dir;
typedef struct{
	pthread_mutex_t mutex;			/* protect the bucket list */
	name_zone_t user_name;			/* user name */
	u16 uga[UGA];					/* uid + gid + acl */
	struct list_head u_list;		/* link to next user_dir */
	struct list_head u_hash;		/* user_dir hash links */
	struct list_head buckets;		/* head of buckets list for this user */
}user_dir_t;
typedef struct{
	pthread_mutex_t mutex;			/* protect the object list */
	name_zone_t bucket_name;		/* bucket name */
	u16 uga[UGA];					/* uid + gid + acl */
	struct list_head b_list;		/* links to next bucket */
	struct list_head b_hash;		/* bucket hash links */
	struct list_head objects;		/* head of objects list in this bucket */
	user_dir_t * user_dir;			/* user_dir which this bucket belongs to */
}bucket_t;
typedef struct _user_object{
	name_zone_t object_name;
	u16 uga[UGA];					/* uid + gid + acl */
	struct list_head o_list;		/* links to next object */
	struct list_head o_hash;		/* object hash links */
	bucket_t * bucket;				/* bucket which this object belongs to */
}object_t;
#define OBJECT_SZ					sizeof(object_t)
#define BUCKET_SZ					sizeof(bucket_t)
#define USER_DIR_SZ					sizeof(user_dir_t)
#define ROOT_DIR_SZ					sizeof(root_dir)
#define for_each_user(lh)			for_each_lhe(lh,&root_ptr->user_dirs)
#define for_each_bucket(lh,user)	for_each_lhe(lh,&user->buckets)
#define for_each_object(lh,bucket)	for_each_lhe(lh,&bucket->objects)
#define for_each_hash(lh,hash)		for_each_lhe(lh,hash)
#define BITS_OF_INT		(8*sizeof(int))
#define OBJ_HASH_BITS	12
#define BKT_HASH_BITS	11
#define USER_HASH_BITS	10
#define OBJ_HASH_NR		(1 << OBJ_HASH_BITS)
#define BKT_HASH_NR		(1 << BKT_HASH_BITS)
#define USER_HASH_NR	(1 << USER_HASH_BITS)
#define U_HASH_MASK		(USER_HASH_NR - 1)
#define B_HASH_MASK		(BKT_HASH_NR - 1)
#define O_HASH_MASK		(OBJ_HASH_NR - 1)
#define OP_WITH_LOCK		00
#define OP_WITHOUT_LOCK		01
extern int init_name_space(void);
extern int put_object(char * object_name,char * bucket_name,char * user_name);
extern int get_object(char * object_name,char * bucket_name,char * user_name);
extern int delete_object(char * object_name,char * bucket_name,char * user_name);

extern int put_bucket(char * bucket_name,char * user_name);
extern int get_bucket(char * bucket_name,char * user_name,char * xml_file);
extern int delete_bucket(char * bucket_name,char * user_name);

extern int put_user(char * user_name);
extern int get_user(char * user_name,char * xml_file,char gu_flag);
extern int delete_user(char * user_name);

extern void get_absolute_path_of_object(object_t * object,char name_buf[]);
extern void parse_path(char *full_path,char **user_name,char **bucket_name,char **obj_name);

/* for debug */
extern void prt_olist(bucket_t * bucket);
extern void prt_blist(user_dir_t * user);
extern void prt_ulist(void);
extern void prt_uhash(void);
extern void prt_bhash(void);
extern void prt_ohash(void);
#define GU_LIST_BUCKETS				00
#define GU_LIST_ALL_BUCKETS_OBJECTS	01
#define GU_FLAG_VALID(guf)	(guf == GU_LIST_ALL_BUCKETS_OBJECTS || \
		guf == GU_LIST_BUCKETS)
#endif
