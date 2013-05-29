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
	char flag;
	char * ptr;	/* starting addr of this name zone */
	name_buf * nb;
	int len;	/* how many bytes in this zone */
	struct list_head t_list;
	struct list_head f_list;
}name_zone;
#define NAME_ZONE_SZ	sizeof(name_zone)
typedef char ** name_zone_t;
#define NAME_ZONE_NULL	(name_zone_t)0
extern struct list_head name_buf_list;
extern void init_name_buf(void);
extern name_zone_t get_name_zone(int len);
extern void put_name_zone(name_zone_t name);
extern void print_name_buf(name_buf * nb);
extern void print_all_name_buf(void);
#endif
