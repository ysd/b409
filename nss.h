#include"global.h"
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
#define _uid(p)			FIELD_OF(p,uga[UID])
#define _gid(p)			FIELD_OF(p,uga[GID])
#define _acl(p)			FIELD_OF(p,uga[ACL])
struct _user_object;
struct _user_bucket;
struct _user_directory;
typedef struct _user_object{
	u8 * o_name;
	u16 uga[UGA];
	struct list_head o_list;				/* next sibling */
	struct list_head o_hash;
	struct _user_bucket * bkt;				/* bucket which contains this file */
}u_obj;
#define OBJ_SZ	sizeof(u_obj)
typedef struct _user_bucket{
	u8 * b_name;
	pthread_mutex_t mutex;
	u16 uga[UGA];
	struct list_head b_objects;				/* first child */
	struct list_head b_list;				/* next sibling */
	struct list_head b_hash;
	struct list_head * object_hashtable;
	struct _user_directory * ud;			/* home dir for this user */
}u_bkt;
#define BKT_SZ	sizeof(u_bkt)
typedef struct _user_directory{
	u8 * u_name;
	pthread_mutex_t mutex;
	u16 uga[UGA];
	struct list_head d_buckets;				/* first child */
	struct list_head d_list;				/* next sibling */
	struct list_head d_hash;
	struct list_head * bucket_hashtable;
}u_dir;
#define UDIR_SZ	sizeof(u_dir)
typedef struct{
	u8 * root;
	pthread_mutex_t mutex;
	u16 uga[UGA];
	struct list_head r_udirs;				/* first user */
	struct list_head * user_hashtable;
}root_dir;
#define ROOT_DIR	sizeof(root_dir)
