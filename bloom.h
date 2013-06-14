/* by Grant Chen
 * 14,June,2013
 * NLO,HUST,Wuhan,China*/
#ifndef _bloom_filter_t_H
#define _bloom_filter_t_H
#include"global.h"
#include"hash.h"
/* one byte for one block */
typedef struct {
	u64 size;	/* bytes in bloom filter */
	u8 * bfz;	/* bloom filter zone */
	u8 hfc;		/* how many hash functions used */
	hash_func_t * hfa;	/* hash func array */
}bloom_filter_t;
#define BLOOM_FILTER_T_SZ	sizeof(bloom_filter_t)

#define INC_REFC(a,n)	(a[n]++)
#define DEC_REFC(a,n)	(a[n])

#define BLOOM_INC	00	/* for operation of increasing reference count */
#define BLOOM_DEC	01	/* for operation of decreasing reference count */
#define BLOOM_CHK	02	/* for operation of checking reference count */
#define bloom_op_flag_valid(bff)	\
	   ((bff) == BLOOM_INC	||	\
		(bff) == BLOOM_DEC  ||	\
		(bff) == BLOOM_CHK)
extern bloom_filter_t * bloom_create(u64 size,u8 hash_nr,...);
extern void bloom_destroy(bloom_filter_t * bloom);
/* increase or decrease reference count by one */
extern int bloom_op(bloom_filter_t *bloom,char * md5,u8 b_flag);

#endif
