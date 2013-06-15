#ifndef _BLK_INX_H
#define _BLK_INX_H
#include"md5.h"
#define INDEX_FILE_DIR				"/mnt/index/"
/* full path : index_file_dir + finger_print_of_file_name + .idx */
#define INDEX_FILE_FULL_PATH_FMT	"%s%s.idx"

static inline void get_index_file_name(char * fp,char * index_file_name)
{
	bzero(index_file_name,MAX_PATH);
	snprintf(index_file_name,MAX_PATH,INDEX_FILE_FULL_PATH_FMT,INDEX_FILE_DIR,fp);
}

#define BLK_DIRTY	0x01
#define BLK_PRESENT	0x02

#define set_blk_d(blk)		((blk)->flag |= BLK_DIRTY)
#define set_blk_p(blk)		((blk)->flag |= BLK_PRESENT)

#define clear_blk_d(blk)	((blk)->flag &= ~BLK_DIRTY)
#define clear_blk_p(blk)	((blk)->flag &= ~BLK_PRESENT)

#define blk_dirty(blk)		((blk)->flag & BLK_DIRTY)
#define blk_present(blk)	((blk)->flag & BLK_PRESENT)
typedef struct{
	char finger_print[MD5_STRING_LEN];	/* finger print of this block */
	char flag;							/* flag of this block,encoding dirty&present flag */
}index_entry_t;
#define BLOCK_INDEX_ENTRY_SZ	sizeof(index_entry_t)

extern int init_index_file(char *fp);
extern int del_index_file(char *fp);
static inline void prt_idxe(index_entry_t * idxe)
{
	printf("#%s	#%s	#%s\n",idxe->finger_print,
			(blk_present(idxe)?"P":"NP"),
			(blk_dirty(idxe)?"DIRTY":"CLEAN"));
}

#endif
