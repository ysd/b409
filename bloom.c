#include"global.h"
#include"bloom.h"
#include"hash.h"
#include"md5.h"
bloom_filter_t *bloom_create(u64 size,u8 hash_nr,...)
{
	u8 i;
	bloom_filter_t *bloom;
	va_list l;
	
	if(!(bloom = malloc(BLOOM_FILTER_T_SZ))) {
		return NULL;
	}
	bzero(bloom,BLOOM_FILTER_T_SZ);
	if(!(bloom->bfz = malloc(size))) {
		goto free_bloom_and_ret;
	}
	/* initialize the bloom zone */
	bzero(bloom->bfz,bloom->size);
	bloom->size = size;
	bloom->funcs = (hashfunc_t*)malloc(HASH_FUNC_T_SZ*hash_nr);
	if(bloom->funcs == NULL){
		goto free_bloom_bfz_and_go_on;
	}
	/* initialize the hash function array */
	va_start(l,hash_nr);
	for(i=0;i<hash_nr;i++){
		bloom->funcs[i] = va_arg(l,hashfunc_t);
	}
	bloom->nfuncs = hash_nr;
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
	free(bloom->funcs);
	free(bloom);
	return;
}

/* int bloom_op(bloom_filter_t *bloom,char *md5,const u8 flag)
 * @bloom : bloom filter to be operated on
 * @md5 : the finger print of the blk
 * @flag : operation flag
 * two kinds of operations : 
 * 1) increase the reference count
 * 2) decrease the reference count
 * return value:
 * both operations return -1 on failure
 * for BLOOM_DEC operation,1 is returned on success
 * BLOOM_INC operation always succedds when none -1 is returned
 * success situation : 
 * 1) 0,when the blk is not originally in cache
 * 2) 1,when the blk is already in cache before BLOOM_INC operation
 * */
int bloom_op(bloom_filter_t *bloom,char *md5,const u8 flag)
{
	int ret = 1;
	u8 i,*bfz;
	u64 idx;
	u32 hv;
	if(!bloom_op_flag_valid(flag)){
		fprintf(stderr,"unrecognized operation flag!\n");
		return -1;
	}
	bfz = bloom->bfz;
	for(i=0;i<bloom->nfuncs;i++){
		hv = (*(bloom->funcs[i]))(md5);
		idx = hv%bloom->size;
		switch(flag){
			case BLOOM_INC:
				if(_REFC(bfz,idx) == MAX_REFC){
					fprintf(stderr,"MAX_REFC has been reached!\n");
					return -1;
				}else if(ret != 0 && _REFC(bfz,idx) == 0){
					/* if any of the reference count == 0
					 * it means that this blk does not exist in cache,
					 * it should be stored in cache as a small blk file,
					 * whose file name is its hash finger print */
					ret = 0;
				}
				INC_REFC(bfz,idx);
				break;
			case BLOOM_DEC:
				if(_REFC(bfz,idx) > 0){
					DEC_REFC(bfz,idx);
				}else{
					/* error situation 
					 * when any of the reference count is <= 0,
					 * which means that the blk does not exist in cache */
					fprintf(stderr,"error!try to decrease the refc for a non-existed blk!\n");
					return -1;
				}
				break;
		}
	}
	return ret;
}
/*  1) >=0 is returned on success
	and the return value is the bytes truely written to cache
 * 2) -1 is returned on failure */
int dedup(bloom_filter_t *bloom,u8 * buf,u32 buflen)
{
	int i,fd;
	char blk_cache_path[MAX_PATH];
	u8 md5c[MD5_CHECKSUM_SZ],md5s[MD5_STRING_LEN];
	md5(buf,buflen,md5c);
	bzero(md5s,MD5_STRING_LEN);
	md5_2_str(md5c,md5s);
	i = bloom_op(bloom,md5s,BLOOM_INC);
	if(i == -1){
		fprintf(stderr,"BLOOM_INC fail!\n");
		return -1;
	}
	if(i == 1){
		/* just return
		   no bytes is written to cache
		 */
		return 0;
	}
	/* store the blk file in CACHE_PATH */
	bzero(blk_cache_path,MAX_PATH);
	snprintf(blk_cache_path,MAX_PATH,"%s%s",CACHE_PATH,md5s);
	fd = open(blk_cache_path,O_CREAT | O_RDWR,0660);
	if(fd == -1){
		perror("open");
		return -1;
	}
	if(write(fd,buf,buflen) != buflen){
		perror("write");
		close(fd);
		return -1;
	}
	close(fd);
	return buflen;
}
/* for debug */
void show_bloom_info(bloom_filter_t * bloom)
{
	u64 i;
	printf("--------- bloom basic information ---------\n");
	printf("-- bloom size	#%ld\n",bloom->size);
	printf("-- bloom nfuncs	#%d\n",bloom->nfuncs);
	printf("-------- bloom filter zone --------\n"); 
	for(i=0;i<bloom->size;i++){
		printf("%d ",_REFC(bloom->bfz,i));
		if((i+1)%64 == 0){
			printf("\n");
		}
	}
	printf("\n");
	return;
}
