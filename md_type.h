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

#define MD_FLAG_IN_CACHE	0x1
#define MD_FLAG_CACHE_DIRTY	0x10
#define is_in_cache(md)				((md)->flag & MD_FLAG_IN_CACHE)
#define is_in_cache_and_dirty(md)	(is_in_cache(md) && \
		((md)->flag & MD_FLAG_CACHE_DIRTY))
#define set_cache_bit(md)	(md)->flag |= MD_FLAG_IN_CACHE
#define clear_cache_bit(md)	(md)->flag &= ~MD_FLAG_IN_CACHE
/* dirty bit is valid only when cache bit is set */
#define set_cache_dirty(md)	(md)->flag |= MD_FLAG_CACHE_DIRTY
#define set_cache_clean(md)	(md)->flag &= ~MD_FLAG_CACHE_DIRTY
typedef struct{
	char rep_ip[IP_LENGTH];
}Replicas;
typedef struct{
	Replicas replica[REPLICA_NUM];
	u64 size;
	time_t atime;
	time_t ctime;
	time_t mtime;
	char flag;
}meta_data_t;
#define MD_SZ	sizeof(meta_data_t)
extern int md_get(char *path,meta_data_t *md);
extern int md_put(char *path,meta_data_t *md);
extern int md_out(char *path);
extern int init_md_of_obj(char *md5s);
extern int de_init_md_of_obj(char *md5s);
#endif
