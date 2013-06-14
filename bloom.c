#include"global.h"
#include"bloom.h"
#include"hash.h"

bloom_filter_t *bloom_create(u64 size,u8 hash_nr,...)
{
	u8 i;
	bloom_filter_t *bloom;
	hash_func_t * hf;
	va_list l;
	
	if(!(bloom = malloc(BLOOM_FILTER_T_SZ))) {
		return NULL;
	}
	bzero(bloom,BLOOM_FILTER_T_SZ);
	if(!(bloom->bfz = malloc(size))) {
		goto free_bloom_and_ret;
	}
	bloom->size = size;
	bloom->hfa = (hash_func_t*)malloc(HASH_FUNC_T_SZ*hash_nr);
	if(bloom->hfa == NULL){
		goto free_bloom_bfz_and_go_on;
	}
	/* initialize the hash function array */
	va_start(l,hash_nr);
	for(i=0;i<hash_nr;i++){
		hf = va_arg(l,hash_func_t);
		bloom->hfa[i] = hf;
	}
	va_end(l);
	goto ret;
free_bloom_bfz_and_go_on:
	free(bloom->bfz);
free_bloom_and_ret:
	free(bloom);
	bloom = NULL;
ret:
	return bloom;
}

void bloom_destroy(bloom_filter_t *bloom)
{
	free(bloom->bfz);
	free(bloom->hfa);
	free(bloom);
}

int bloom_op(bloom_filter_t *bloom,char *md5,u8 flag)
{
	u8 i;
	u32 hv;
	if(!bloom_op_flag_valid(flag)){
		return 1;
	}
	for(i=0;i<bloom->hfc;i++){
		hv = (*bloom->hfa[i])(md5);
	}
	return 0;
}
int bloom_inc_refc(bloom_filter_t *bloom, int n, ...)
{
	va_list l;
	uint32_t pos;
	int i;

	va_start(l, n);
	for (i = 0; i < n; i++) {
		pos = va_arg(l, uint32_t);
		SETBIT(bloom->a, pos % bloom->asize);
	}
	va_end(l);

	return 0;
}

int bloom_check(bloom_filter_t *bloom, int n, ...)
{
	va_list l;
	uint32_t pos;
	int i;

	va_start(l, n);
	for (i = 0; i < n; i++) {
		pos = va_arg(l, uint32_t);
		if(!(GETBIT(bloom->a, pos % bloom->asize))) {
			return 0;
		}
	}
	va_end(l);

	return 1;
}
