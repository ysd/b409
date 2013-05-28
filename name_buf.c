#include"global.h"
#include"name_buf.h"
static struct list_head name_buf_list;
static void init_name_buf_list(void)
{
	list_head_init(&name_buf_list);
}
static void add_name_zone_to_free_list(struct list_head * start_lh,
		struct list_head * end_lh,
		struct list_head * free_name_zone)
{
	struct list_head * lh;
	name_zone *nz1,*nz2 = CONTAINER_OF(free_name_zone,name_zone,f_list);
	for_each_list_head_between(lh,start_lh,end_lh){
		nz1 = CONTAINER_OF(lh,name_zone,f_list);
		/* free zone are sorted by len in decend order */
		if(nz2->len >= nz1->len){break;}
	}
	list_add_tail(free_name_zone,lh);
	return;
}
static name_zone * new_name_zone(char * ptr,name_buf * nb,int len,u8 flag)
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
		nb->free = NAME_BUF_LEN;
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
static char ** get_name_zone(int len)
{
	name_buf * nb;
	name_zone * nz,*nnz;
	struct list_head * lh,*flh;
retry:
	/* look up all the name_bufs */
	for_each_lhe(lh,&name_buf_list){
		nb = CONTAINER_OF(lh,name_buf,nb_list);
		/* every name_zone in a paticular name_buf */
		for_each_lhe(flh,&nb->free_zone){
			nz = CONTAINER_OF(flh,name_zone,f_list);
			/* case 1 : space is not enough in this name_zone */
			if(nz->len < len + 1){
				continue;
			}
			if(nz->len == len + 1){
				/* case 2 : right ok! remove nz from nb->free_zone */
				list_del(&nz->f_list);
				list_head_init(&nz->f_list);
				nz->flag = NAME_ZONE_TAKEN;
				return &nz->ptr;
			}
			/* case 3 : more bytes than needed in this name_zone */
			nzz = new_name_zone(nz->ptr,nb,len + 1,NAME_ZONE_TAKEN);
			if(nzz == NULL){
				return NULL;
			}
			/* add nzz to the total name_zone list
			 * nzz should be right before nz,
			 * so here we use list_add_tail with nz->t_list */
			list_add_tail(&nzz->t_list,&nz->t_list);
			/* adjust len & ptr */
			nz->len -= (len + 1);
			nz->ptr += (len + 1);
			/* as length of this name_zone has changed,
			 * place it to the appropriately */
			list_del(&nz->f_list);
			/* nz->f_list still valid */
			/* as in the left of nz,all the len is bigger than nz,
			 * we search the new position for nz from the next one */
			add_name_zone_to_free_list(&nz->f_list,&nb->free_zone,&nz->f_list);
			return &nzz->ptr;
		}
	}
	/* if we got here,some more name_buf is needed */
	if(more_name_buf() == 0){
		goto retry;
	}
	return NULL;
}
static void free_name_zone(char ** name)
{
	/* nz is the name_zone to be freed */
	name_zone *nz_pn,* nz = CONTAINER_OF(name,name_zone,ptr);
	name_buf * nb = nz->nb;
	if(nz->t_list->prev != &nb->total_zone){
		nz_pn = CONTAINER_OF(nz->t_list->prev,name_zone,t_list);
		if(nz_pn->flag == NAME_ZONE_FREE){
			/* join with prev one */
			if(nz_pn->ptr + nz_pn->len != nz->ptr){
				fprintf(stderr,"free : something is wrong!\n");
				return;
			}
			nz_pn->len += nz->len;
			/* delete nz from total zone list and free it */
			list_del(&nz->t_list);
			free_name_zone(nz);
			nz = nz_pn;
		}
	}
	if(nz->t_list->next != &nb->total_zone){
		nz_pn = CONTAINER_OF(nz->t_list->next,name_zone,t_list);
		if(nz_pn->flag == NAME_ZONE_FREE){
			if(nz->ptr + nz->len != nz_pn->ptr){
				fprintf(stderr,"free : something is wrong!\n");
				return;
			}
			nz_pn->len += nz->len;
			nz_pn->ptr = nz->ptr;
			list_del(&nz->t_list);
			free_name_zone(nz);
			nz = nz_pn;
		}
	}
	if(nz->flag == NAME_ZONE_TAKEN){
		nz->flag = NAME_ZONE_FREE;
		add_name_zone_to_free_list(&nb->free_zone,&nb->free_zone,nz->f_list);
	}
	return;
}
