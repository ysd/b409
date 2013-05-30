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
	typeof((p)->uid[0]) _v = (v);	\
	(p)->uga[UID] = _v;	\
}while(0)
#define set_gid(p,v)		do{	\
	typeof((p)->uid[0]) _v = (v);	\
	(p)->uga[GID] = _v;	\
}while(0)
#define set_acl(p,v)		do{	\
	typeof((p)->uid[0]) _v = (v);	\
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
	struct list_head b_hash			/* bucket hash links */
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
#endif
