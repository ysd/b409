#ifndef _NSS_H
#define _NSS_H
#include"global.h"
#include"name_buf.h"
#define ROOT_DIR	"/"
#define SHARED_DIR	"shared"
/* /user_dir/bucket/object */
#define ABS_PATH_FMT	"/%s/%s/%s"
enum{
	UID=0,
	GID,
	ACL,
	UGA
};
#define SU_UID	0
#define SU_GID	0
#define O_R	0400
#define O_W	0020
#define O_X	0001
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
	(p)->uga[UID] = (v);	\
}while(0)
#define set_gid(p,v)		do{	\
	(p)->uga[GID] = (v);	\
}while(0)
#define set_acl(p,v)		do{	\
	(p)->uga[ACL] = (v);	\
}while(0)
typedef struct{
	pthread_mutex_t mutex;
	u16 uga[UGA];							/* uid + gid + acl */
	struct list_head user_dirs;				/* head of user_dir list */
	struct list_head * ud_hashtable;		/* user_dir hash table */
}root_dir;
typedef struct{
	pthread_mutex_t mutex;					/* mutex protecting this user_dir */
	name_zone_t user_name;					/* user name */
	u16 uga[UGA];							/* uid + gid + acl */
	struct list_head d_list;				/* link to next user_dir */
	struct list_head d_hash;				/* user_dir hash links */
	struct list_head buckets;				/* head of buckets list for this user */
	struct list_head * bucket_hashtable;	/* buckets hash table */
}user_dir_t;
typedef struct{
	pthread_mutex_t mutex;					/* mutex protecting this bucket */
	name_zone_t bucket_name;				/* bucket name */
	u16 uga[UGA];							/* uid + gid + acl */
	struct list_head b_list;				/* links to next bucket */
	struct list_head b_hash;				/* bucket hash links */
	struct list_head objects;				/* head of objects list in this bucket */
	struct list_head * object_hashtable;	/* objects hash table */
	user_dir_t * user_dir;					/* user_dir which this bucket belongs to */
}bucket_t;
typedef struct _user_object{
	name_zone_t object_name;
	u16 uga[UGA];							/* uid + gid + acl */
	struct list_head o_list;				/* links to next object */
	struct list_head o_hash;				/* object hash links */
	bucket_t * bucket;						/* bucket which this object belongs to */
}object_t;
#define OBJECT_SZ		sizeof(object_t)
#define BUCKET_SZ		sizeof(bucket_t)
#define USER_DIR_SZ		sizeof(user_dir_t)
#define ROOT_DIR_SZ		sizeof(root_dir)
#endif
