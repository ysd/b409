#include"memalloc.h"
#define DBGMSG
#undef DBGMSG
static Header base;
static Header * freep = NULL;
void zz_free(void * ap)
{
	Header * bp,*p;
	bp = (Header*)ap - 1;
	p = freep;
#ifdef DBGMSG
	printf("to be freed : block #%0x units #%d\n",bp,szoffb(bp));
	printf("first find position that the block should be placed\n");
#endif
	while(1){
#ifdef DBGMSG
		printf("-------\np #%0x\nnp #%0x\n",p,nfp(p));
#endif
		if(bp > p && bp < nfp(p)){
#ifdef DBGMSG
			printf("block should be placed between p and nfp(p)\n");
#endif
			break;
		}
		if(p >= nfp(p) && (bp > p || bp < nfp(p))){
#ifdef DBGMSG
			printf("block should be placed at the end or the start\n");
#endif
			break;
		}
		p = nfp(p);
	}
	if(bp + szoffb(bp) == nfp(p)){
#ifdef DBGMSG
		printf("join with next free block\n");
#endif
		szoffb(bp) += szoffb(nfp(p));
		nfp(bp) = nfp(nfp(p));
	}else{
#ifdef DBGMSG
		printf("no need to join with next free block\n");
#endif
		nfp(bp) = nfp(p);
	}
	if(p + szoffb(p) == bp){
#ifdef DBGMSG
		printf("join with previous free block\n");
#endif
		szoffb(p) += szoffb(bp);
		nfp(p) = nfp(bp);
	}else{
#ifdef DBGMSG
		printf("no need to join with previous free block\n");
#endif
		nfp(p) = bp;
	}
	freep = p;
	return;
}
static Header * morecore(u32 nunits)
{
	char * cp;
	Header * up;
	if(++nunits < MINALLOC){
		nunits = MINALLOC;
	}
	cp = (char *)sbrk(nunits*HEADER_SZ);
	if(cp == (char *)-1){
		return NULL;
	}
	up = (Header*)cp;
	szoffb(up) = nunits - 1;
	zz_free((void*)(up+1));
	return freep;
}
void * zz_malloc(u32 nbytes)
{
	Header * p,*pre;
	u32 nunits = nbytes/HEADER_SZ + (nbytes%HEADER_SZ == 0?0:1);
#ifdef DBGMSG
	printf("#%d bytes requested\n",nbytes);
	printf("#%d units need to be allocated\n",nunits);
#endif
	if((pre = freep) == NULL){
		nfp(&base) = freep = pre = &base;
		szoffb(&base) = 0;
#ifdef DBGMSG
		printf("first use,free list is null!\n");
		printf("base initialized with sz == %d\n",szoffb(&base));
#endif
	}
#ifdef DBGMSG
	printf("--------------------now search the free block list--------------------\n");
#endif
	do{
		p = nfp(pre);
#ifdef DBGMSG
		printf("FREE BLOCK WITH UNITS %d\n",szoffb(p));
#endif
		if(szoffb(p) >= nunits){
			if(szoffb(p) == nunits){
#ifdef DBGMSG
				printf("exactly ok\n");
#endif
				nfp(pre) = nfp(p);
			}else{
#ifdef DBGMSG
				printf("bigger then needed\n");
				printf("----------------------zz_malloc ok!--------------------\n");
#endif
				/* one more block needed for malloced block's meta info */
				szoffb(p) -= (nunits + 1);
				p += (szoffb(p) + 1);
				szoffb(p) = nunits;/* why? */
			}
			freep = pre;
			return (void*)(p+1);
		}
#ifdef DBGMSG
		printf("space not enough\n");
#endif
		if(p == freep){
#ifdef DBGMSG
			printf("--------------------------------------------------------------------------\n");
			printf("and all free blocks have been checked,more space need be allocated from OS\n");
#endif
			if((p = morecore(nunits)) == NULL){
#ifdef DBGMSG
				printf("from OS : no enough space!malloc will fail\n");
#endif
				return NULL;
			}
#ifdef DBGMSG
			printf("new space has been added to free list,search again\n");
			printf("--------------------------------------------------------------------------\n");
#endif
		}
		pre = p;
		p = nfp(p);
	}while(1);
}
static void print_free(Header * p)
{
	Header * start = (p == NULL?freep:p);
	Header * c = start;
	u32 i = 0;
	printf("---------------------- free list start --------------------\n");
	do{
		printf("FREE BLOCK #%d : start #%0x	units #%d\n",i++,c,szoffb(c));
		c = nfp(c);
	}while(c != start);
	printf("---------------------- free list end --------------------\n");
	return;
}
int main()
{
	u32 alloc[100];
	u8 * pa[100];
	u32 i;
	Header * h;
	for(i = 0;i < 100;i++){
		srand(time(NULL) + i);
		alloc[i] = rand()%1000;
		printf("%d ",alloc[i]);
	}
	printf("\n");
	for(i = 0;i < 100;i++){
		pa[i] = zz_malloc(alloc[i]);
		/* screw meta info of the list 
		 * this would be fatal */
		bzero((void*)(pa[i]+alloc[i]-4),4);
		print_free(NULL);
	}
	for(i = 0;i < 100;i++){
		h = (Header*)pa[i] - 1;
		printf("%d ",szoffb(h));
	}
	for(i = 0;i < 50;i++){
		zz_free(pa[i]);
		print_free(NULL);
	}
	printf("\n");
	return 0;
}
