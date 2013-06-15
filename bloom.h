/* by Grant Chen
 * 14,June,2013
 * NLO,HUST,Wuhan,China*/
#ifndef _BLOOM_H
#define _BLOOM_H
#include"global.h"
#include"hash.h"
#define BLOOM_FILTER_ZONE_SZ	_1K
/* one byte for one block */
typedef struct {
	u64 size;	/* bytes in bloom filter */
	u8 * bfz;	/* bloom filter zone */
	u8 nfuncs;	/* how many hash functions used */
	hashfunc_t * funcs;	/* hash func array */
}bloom_filter_t;
#define BLOOM_FILTER_T_SZ	sizeof(bloom_filter_t)

#define MAX_REFC		255 
#define INC_REFC(a,n)	(a[n]++)
#define DEC_REFC(a,n)	(a[n]--)
#define _REFC(a,n)		(a[n])

#define BLOOM_INC	00	/* for operation of increasing reference count */
#define BLOOM_DEC	01	/* for operation of decreasing reference count */
#define bloom_op_flag_valid(bff)	\
	   ((bff) == BLOOM_INC	||	(bff) == BLOOM_DEC) 
extern bloom_filter_t * bloom_create(u64 size,u8 hash_nr,...);
extern void bloom_destroy(bloom_filter_t * bloom);
/* increase or decrease reference count by one */
extern int bloom_op(bloom_filter_t *bloom,char * md5,u8 b_flag);

#define FIX_BLK_SZ	(4*_1K)
extern int dedup(bloom_filter_t *bloom,u8 * buf,u32 buflen);
extern void show_bloom_info(bloom_filter_t * bloom);
#endif
