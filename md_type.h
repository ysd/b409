#ifndef _MD_H
#define _MD_H
#define META_DATA_DB	"meta_data.tch"
#define FILE_NODE_DB	"file_node.tch"
#define IO_NODE_DB		"io_node.tch"
#define IO_DATA_DB		"io_data.tch"
typedef enum{
	MD = 0,
	ION,
	IOD,
}tc_t;
#define REPLICA_NUM 5
#define MAX_IO_ON_A_FILE 50
#define IP_LENGTH 16
#define MD_CLEAN	00
#define MD_DIRTY	01
typedef struct{
	char rep_ip[IP_LENGTH];
}Replicas;
typedef struct{
	time_t atime;
	time_t ctime;
	time_t mtime;
	u64 size;
	Replicas replica[REPLICA_NUM];
	/* for update dtc file */
	char dirty;
    char io_node_head[MAX_ION_PATH]; /* point to struct IO_Node */
	char io_node_tail[MAX_ION_PATH]; /* point to struct IO_Node */
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
