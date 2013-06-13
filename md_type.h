#ifndef _MD_H
#define _MD_H
#define INDEX_TABLE_FILE_NAME_LEN	64
#define INDEX_TABLE_FILE_FMT		"%s.index_table_file"
#define META_DATA_DB				"meta_data.tch"
typedef enum{
	MD = 0,
	ION,
	IOD,
}tc_t;
#define REPLICA_NUM	3
#define IP_LENGTH	INET_ADDRSTRLEN

#define MD_FLAG_CACGE_DIRTY	0x10
#define MD_FLAG_CACHE_CLEAN	00
#define MD_FLAG_IN_CACHE	01
#define MD_FLAG_IN_DATACT	00
#define is_in_cache(md)	((md)->md_flag & MD_FLAG_IN_CACHE)
#define set_in_data_center(md)		do{	\
	typeof(md) _md = md;	\
	_md->md_flag = 0;	\
}while(0)
#define set_in_cache(md)			do{	\
	typeof(md) _md = md;	\
	_md->md_flag = 0;	\
	_md->md_flag |= MD_FLAG_IN_CACHE;	\
}while(0)
typedef struct{
	char rep_ip[IP_LENGTH];
}Replicas;
typedef struct{
	Replicas replica[REPLICA_NUM];
	u64 size;
	time_t atime;
	time_t ctime;
	time_t mtime;
	char md_flag;
}meta_data_t;
#define MD_SZ	sizeof(meta_data_t)
_PROTOTYPE(int md_get,(char *path,meta_data_t *md));
_PROTOTYPE(int md_put,(char *path,meta_data_t *md));
_PROTOTYPE(int md_out,(char *path));
#endif
