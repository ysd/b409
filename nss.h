#include"global.h"
#define ROOT_DIR	"/"
#define SHARED_DIR	"shared"
#define FC_OF(p)			FIELD_OF(p,fc)
#define NS_OF(p)			FIELD_OF(p,ns)
#define BUCKET_OF_OBJ(p)	FIELD_OF(p,bkt)
#define UDIR_OF_BUCKET(p)	FIELD_OF(p,ud)
#define ROOT_DIR_OF_USER	(&root_dir)
#define NAME_OF_OBJ(p)		FIELD_OF(p,object_name)
#define NAME_OF_BKT(p)		FIELD_OF(p,bucket_name)
#define NAME_OF_UDIR(p)		FIELD_OF(p,user_name)
enum{
	UID=0,
	GID,
	ACL,
	UGA
};
#define _UID(p)			FIELD_OF(p,uga[UID])
#define _GID(p)			FIELD_OF(p,uga[GID])
#define _ACL(p)			FIELD_OF(p,uga[ACL])
struct _user_object;
struct _user_bucket;
struct _user_directory;
typedef struct _user_object{
	u8 * object_name;
	u32 uga[UGA];
	struct _user_object * ns;			/* next sibling */
	struct _user_bucket * bkt;			/* bucket which contains this file */
}u_obj;
typedef struct _user_bucket{
	u8 * bucket_name;
	u32 uga[UGA];
	struct _user_object * fc;			/* first child */
	struct _user_bucket * ns;			/* next sibling */
	struct _user_directory * ud;		/* home dir for this user */
}u_bkt;
typedef struct _user_directory{
	u8 * user_name;
	u32 uga[UGA];
	struct _user_bucket * fc;			/* first child */
	struct _user_directory * ns;		/* next sibling */
}u_dir;
struct{
	u8 * root;
	struct _user_directory * fc;		/* first user */
}root_dir;
