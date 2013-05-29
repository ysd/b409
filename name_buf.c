#include"global.h"
#include"utility.h"
#include"name_buf.h"
/* to protect the name_buf when get_name_zone and put_name_zone is called */
static pthread_mutex_t name_buf_mutex = PTHREAD_MUTEX_INITIALIZER;
struct list_head name_buf_list;
void init_name_buf(void)
{
	list_head_init(&name_buf_list);
}
static void add_name_zone_to_free_list(struct list_head * start_lh,
		struct list_head * end_lh,
		struct list_head * free_name_zone)
{
	struct list_head * lh;
	name_zone *nz1,*nz2 = container_of(free_name_zone,name_zone,f_list);
	for_each_list_head_between(lh,start_lh,end_lh){
		nz1 = container_of(lh,name_zone,f_list);
		/* free zone are sorted by len in decend order */
		if(nz2->len >= nz1->len){break;}
	}
	list_add_tail(free_name_zone,lh);
	return;
}
static name_zone * new_name_zone(char * ptr,name_buf * nb,int len,char flag)
{
	name_zone * nz = (name_zone*)malloc(NAME_ZONE_SZ);
	if(nz != NULL){
		nz->flag = flag;
		nz->ptr = ptr;
		nz->nb = nb;
		nz->len = len;
		list_head_init(&nz->t_list);
		list_head_init(&nz->f_list);
	}
	return nz;
}
static void free_name_zone(name_zone * nz)
{
	free(nz);
}
static name_buf * new_name_buf(void)
{
	name_zone * nz;
	name_buf * nb = (name_buf*)malloc(NAME_BUF_SZ);
	if(nb != NULL){
		nb->size = NAME_BUF_LEN;
		nb->free = nb->size;
		nb->buf = (char*)malloc(nb->size);
		if(nb->buf == NULL){
			nb = NULL;
			goto r2;
		}
		list_head_init(&nb->free_zone);
		list_head_init(&nb->total_zone);
		list_head_init(&nb->nb_list);
		/* initialize as a free zone */
		nz = new_name_zone(nb->buf,nb,nb->size,NAME_ZONE_FREE);
		if(nz == NULL){
			nb = NULL;
			goto r1;
		}
		list_add(&nb->total_zone,&nz->t_list);
		add_name_zone_to_free_list(&nb->free_zone,&nb->free_zone,&nz->f_list);
	}
	goto ret;
r1:
	free(nb->buf);
r2:
	free(nb);
ret:
	return nb;
}
static void free_name_buf(name_buf * nb)
{
	free(nb->buf);
	free(nb);
}
static int more_name_buf(void)
{
	name_buf * nb = new_name_buf();
	if(nb == NULL){
		return 1;
	}
	list_add(&nb->nb_list,&name_buf_list);
	return 0;
}
name_zone_t get_name_zone(int len)
{
	/* get a name zone for a given char array with len 
	 * ANY ERROR DURING THIS ROUTINE WILL LEAD TO THE FAILURE */
	name_zone_t rtnz = NAME_ZONE_FREE;
	int bytes_needed = len + 1;
	name_buf * nb;
	name_zone * nz,*nzz;
	struct list_head * lh,*flh;
	if(pthread_mutex_lock(&name_buf_mutex) != 0){
		perror("pthread_mutex_lock");
		goto ret;
	}
retry:
	/* look up all the name_bufs */
	for_each_lhe(lh,&name_buf_list){
		nb = container_of(lh,name_buf,nb_list);
		if(bytes_needed > nb->free){
			continue;
		}
		/* every name_zone in a paticular name_buf */
		for_each_lhe(flh,&nb->free_zone){
			nz = container_of(flh,name_zone,f_list);
			/* case 1 : space is not enough in this name_zone */
			if(nz->len < bytes_needed){
				continue;
			}
			if(nz->len == bytes_needed){
				/* case 2 : right ok! remove nz from nb->free_zone */
				list_del(&nz->f_list);
				list_head_init(&nz->f_list);
				nz->flag = NAME_ZONE_TAKEN;
				nb->free -= bytes_needed;
				rtnz = &nz->ptr;
				goto unlock_and_ret;
			}
			/* case 3 : more bytes than needed in this name_zone */
			nzz = new_name_zone(nz->ptr,nb,bytes_needed,NAME_ZONE_TAKEN);
			if(nzz == NULL){
				goto unlock_and_ret;
			}
			/* add nzz to the total name_zone list
			 * nzz should be right before nz,
			 * so here we use list_add_tail with nz->t_list */
			list_add_tail(&nzz->t_list,&nz->t_list);
			/* adjust len & ptr */
			nz->len -= bytes_needed;
			nz->ptr += bytes_needed;
			/* as length of this name_zone has changed,
			 * place it appropriately */
			list_del(&nz->f_list);
			/* nz->f_list still valid */
			/* as in the left of nz,all the len is bigger than nz,
			 * we search the new position for nz from the next one */
			add_name_zone_to_free_list(&nz->f_list,&nb->free_zone,&nz->f_list);
			nb->free -= bytes_needed;
			rtnz = &nzz->ptr;
			goto unlock_and_ret;
		}
	}
	/* if we got here,some more name_buf is needed */
	if(more_name_buf() == 0){
		goto retry;
	}
unlock_and_ret:
	if(pthread_mutex_unlock(&name_buf_mutex) != 0){
		perror("pthread_mutex_unlock");
		rtnz = NAME_ZONE_NULL;
	}
ret:
	return rtnz;
}
void put_name_zone(name_zone_t name)
{
	/* nz is the name_zone to be freed */
	name_zone *nz_pn,*nz;
	name_buf * nb;
	if(pthread_mutex_lock(&name_buf_mutex) != 0){
		perror("pthread_mutex_lock");
		goto ret;
	}
	nz = container_of(name,name_zone,ptr);
	nb = nz->nb;
	if(nz->flag == NAME_ZONE_FREE){
		fprintf(stderr,"already freed!\n");
		goto unlock_and_ret;
	}
	if(nz->t_list.prev != &nb->total_zone){
		nz_pn = container_of(nz->t_list.prev,name_zone,t_list);
		if(nz_pn->flag == NAME_ZONE_FREE){
			/* join with prev one */
			if(nz_pn->ptr + nz_pn->len != nz->ptr){
				fprintf(stderr,"free : something is wrong!\n");
				goto unlock_and_ret;
			}
			nz_pn->len += nz->len;
			/* delete nz from total zone list and free it */
			list_del(&nz->t_list);
			free_name_zone(nz);
			nz = nz_pn;
			/* delete this free name_zone */
			list_del(&nz->f_list);
		}
	}
	if(nz->t_list.next != &nb->total_zone){
		nz_pn = container_of(nz->t_list.next,name_zone,t_list);
		if(nz_pn->flag == NAME_ZONE_FREE){
			if(nz->ptr + nz->len != nz_pn->ptr){
				fprintf(stderr,"free : something is wrong!\n");
				goto unlock_and_ret;
			}
			nz_pn->len += nz->len;
			nz_pn->ptr = nz->ptr;
			list_del(&nz->t_list);
			free_name_zone(nz);
			nz = nz_pn;
			/* delete this free name_zone */
			list_del(&nz->f_list);
		}
	}
	nz->flag = NAME_ZONE_FREE;
	add_name_zone_to_free_list(&nb->free_zone,&nb->free_zone,&nz->f_list);
unlock_and_ret:
	if(pthread_mutex_unlock(&name_buf_mutex) != 0){
		perror("pthread_mutex_unlock");
	}
ret:
	return;
}
/* for debug */
void print_name_buf(name_buf * nb)
{
	struct list_head * l;
	name_zone * nz;
	printf("######## name_buf #%0x #%d\n--- free_zone_list ---\n",nb->buf,nb->size);
	for_each_lhe(l,&nb->free_zone){
		nz = container_of(l,name_zone,f_list);
		printf("#%0x #%4d #%c -- ",nz->ptr,nz->len,(nz->flag==NAME_ZONE_FREE?'F':'T'));
	}
	printf("\n--- total_zone_list ---\n");
	for_each_lhe(l,&nb->total_zone){
		nz = container_of(l,name_zone,t_list);
		printf("#%0x #%4d #%c -- ",nz->ptr,nz->len,(nz->flag==NAME_ZONE_FREE?'F':'T'));
	}
	printf("\n");
	return;
}
void print_all_name_buf(void)
{
	name_buf * nb;
	struct list_head * l;
	for_each_lhe(l,&name_buf_list){
		nb = container_of(l,name_buf,nb_list);
		print_name_buf(nb);
	}
	return;
}
/*
#define ALLOC_ELEM	20
#define BIGGGG		1000
int main()
{
	int a[ALLOC_ELEM],i,j,count;
	name_zone_t n[ALLOC_ELEM],name;
	name_zone * nz;
	name_buf * nb;
	rand_generator(a,ALLOC_ELEM);
	init_name_buf();
	for(i=0;i<ALLOC_ELEM;i++){
		printf("************************* allocate\t#%4d bytes\n",a[i]);
		n[i] = get_name_zone(a[i]);
		nz = container_of(n[i],name_zone,ptr);
		nb = nz->nb;
		print_name_buf(nb);
	}
	printf("--------------------------------- alloc end ---------------------------\n");
	print_all_name_buf();
	printf("-------------------------------- now free --------------------------------------\n");
	count = ALLOC_ELEM;
	for(i=BIGGGG;i>0;i--){
		srand(time(NULL) + i);
		j = rand()%ALLOC_ELEM;
		if(n[j] != NULL){
			nz = container_of(n[j],name_zone,ptr);
			nb = nz->nb;
			print_name_buf(nb);
			printf("**************************** free #%3d bytes in name_zone #%0x of name_buf #%0x\n",nz->len,nz->ptr,nb->buf);
			put_name_zone(n[j]);
			n[j] = NULL;
			print_name_buf(nb);
			if(--count == 0){
				break;
			}
		}
	}
	printf("--------------------------------- free end ---------------------------\n");
	print_all_name_buf();
	printf("i == %d\n",i);
	return 0;
}
*/
