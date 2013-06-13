#ifndef _MD_H
#define _MD_H
#define INDEX_TABLE_FILE_NAME_LEN	64
#define INDEX_TABLE_FILE_FMT		"%s.index_table_file"
#define META_DATA_DB	"meta_data.tch"
#define FILE_NODE_DB	"file_node.tch"
#define IO_NODE_DB		"io_node.tch"
#define IO_DATA_DB		"io_data.tch"
typedef enum{
	MD = 0,
	ION,
	IOD,
}tc_t;
#define REPLICA_NUM	3
#define IP_LENGTH	INET_ADDRSTRLEN

#ifdef OLD_VERSION_SUPER_PROCESS
/* only used in old version */
#define MD_CLEAN	00
#define MD_DIRTY	01
#endif

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
    char io_node_head[MAX_ION_PATH]; /* point to struct IO_Node */
	char io_node_tail[MAX_ION_PATH]; /* point to struct IO_Node */
	Replicas replica[REPLICA_NUM];
	u64 size;
	time_t atime;
	time_t ctime;
	time_t mtime;
	char md_flag;
}Meta_Data;
#define MD_SZ	sizeof(Meta_Data)
/********************* io node ************************/
#define IO_READ		00
#define IO_WRITE	01
#define IO_CACHE	00
#define IO_DATA		01
typedef struct
{
    time_t modification_time;
    u64 offset;	/* offset of this io */
	u32 length;	/* how many bytes in this io */
    char pre[MAX_ION_PATH];
    char next[MAX_ION_PATH];
}IO_Node;
#define ION_SZ	sizeof(IO_Node)
_PROTOTYPE(int md_get,(char *path,Meta_Data *md));
_PROTOTYPE(int md_put,(char *path,Meta_Data *md));
_PROTOTYPE(int md_out,(char *path));
_PROTOTYPE(int ion_get,(char *key,IO_Node *io_node));
_PROTOTYPE(int ion_put,(char *key,IO_Node *io_node));
_PROTOTYPE(int ion_out,(char *key));
_PROTOTYPE(int iod_get,(char *key,char *io_data));
_PROTOTYPE(int iod_put,(char *key,char *io_data,int len));
_PROTOTYPE(int iod_out,(char *key));
#endif
