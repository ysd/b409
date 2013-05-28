#ifndef _NAME_BUF
#define _NAME_BUF
#include"list_head.h"
#define NAME_BUF_LEN	4096
#define NAME_ZONE_TAKEN	01
#define NAME_ZONE_FREE	00
/* for */
#define NAME_ZONE_MIN	2
typedef struct _name_buf{
	int size;	/* total size of this zone */
	int free;	/* free bytes in this zone */
	char * buf;	/* bug region */
	struct list_head free_zone;
	struct list_head total_zone;
	struct list_head nb_list;
}name_buf;
#define NAME_BUF_SZ	sizeof(name_buf)
typedef struct _name_zone{
	u8 flag;
	char * ptr;	/* starting addr of this name zone */
	name_buf * nb;
	int len;	/* how many bytes in this zone */
	struct list_head t_list;
	struct list_head f_list;
}name_zone;
#define NAME_ZONE_SZ	sizeof(name_zone)
#endif
