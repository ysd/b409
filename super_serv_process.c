#include"global.h"
#include"md_type.h"
#include"utility.h"
#include"list_head.h"
#define DBGMSG
typedef struct _file_node{
	char file_name[MAX_PATH];
	struct list_head f_list;
}file_node;
#define FN_SZ	sizeof(file_node)
static pthread_mutex_t file_node_list_mutex = PTHREAD_MUTEX_INITIALIZER;	/* protect file_node_list */
static pthread_mutex_t md_mutex = PTHREAD_MUTEX_INITIALIZER;				/* protect meta data & io_node list */
static pthread_cond_t file_node_list_cond = PTHREAD_COND_INITIALIZER;		/* for update dtc file */
struct list_head file_node_list_head;
static list_head * file_node_list_head_ptr;	
static list_head * update;													/* now updating */
#define for_each_file(l)			for(l=file_node_list_head_ptr->next;l!=file_node_list_head_ptr;l=l->next)
#define FILE_NODE_LIST_NULL			list_empty(file_node_list_head_ptr)
#define file_node_of(l)				container_of(l,file_node,f_list)
static void init_fn_list(void)
{
	update = NULL;
	file_node_list_head_ptr = &file_node_list_head;
	list_head_init(file_node_list_head_ptr);
	return;
}
static inline file_node * prev_node_of_me(file_node * fn)
{
	if(fn == NULL || fn->f_list.prev == file_node_list_head_ptr){
		return NULL;
	}
	return file_node_of(fn->f_list.prev);
}
static void print_file_node_list(void)
{
	file_node * fn;
	printf("-------------- file_node_list start -------------\n");
	if(update != NULL){
		fn = file_node_of(update);
		printf("now updating file #%s\n",fn->file_name);
	}
	for_each_fn_in_fnlist(fn){
		printf("fn->file_name\t#%s\n",fn->file_name);
	}
	printf("--------------- file_node_list end --------------\n");
	return;
}
#define INIT_FILE_NODE(fn)	do{\
	fn->prev = NULL;\
	fn->next = NULL;\
}while(0)
static file_node * new_file_node(char * file_name)
{
	file_node * fn;
	int len = strlen(file_name);
	if((fn = (file_node*)malloc(FN_SZ)) == NULL){
		perror("file_node malloc");
		return NULL;
	}
	INIT_FILE_NODE(fn);
	strncpy(fn->file_name,file_name,len);
	return fn;
}
static void delete_file_node(file_node * fn)
{
	free(fn);
	return;
}
static file_node * get_fn(char * file_name)
{
	int len = strlen(file_name);
	file_node ** fnp;
	for(fnp=&file_node_list_head;*fnp;fnp=&((*fnp)->next)){
		if(strncmp(file_name,(*fnp)->file_name,len) == 0){
			break;
		}
	}
	return *fnp;
}
static void rm_fn_from_list(file_node * fn)
{
	if(fn->prev){
		fn->prev->next = fn->next;
	}
	if(fn->next){
		fn->next->prev = fn->prev;
	}
	if(fn == file_node_list_head){
		file_node_list_head = fn->next;
	}
	if(fn == update){
		update = fn->prev;
	}
	INIT_FILE_NODE(fn);
	return;
}
static void add2file_node_list_head(char * file_name)
{
	file_node * fn = get_fn(file_name);
	if(fn == NULL){
		fn = new_file_node(file_name);
	}
	fn->prev = NULL;
	if(FILE_NODE_LIST_NULL){
		fn->next = NULL;
	}else{
		fn->next = file_node_list_head->next;
		file_node_list_head->prev = fn;
	}
	file_node_list_head = fn;
	if(update == NULL){
		update = fn;
	}
	return;
}
/* for debug */
static void prtion(IO_Node * ion)
{
	printf("---ion s---\n");
	printf("ion->off\t#%lu\n",ion->offset);
	printf("ion->len\t#%u\n",ion->length);
	printf("ion->next\t#%s\n",ion->next);
	printf("ion->prev\t#%s\n",ion->pre);
	printf("ion->time\t#%u\n",ion->modification_time);
	printf("---ion e---\n");
	return;
}
static void print_ion(char * path)
{
    Meta_Data md;
    IO_Node ion;
	int len,dlen;
    char p[MAX_ION_PATH];
	char * io_data;
	printf("-------------- file %s start ------------\n",path);
	bzero(&md,MD_SZ);
	md_get(path,&md);
	if(md.dirty == MD_CLEAN){
		printf("md.dirty\t%s\n","CLEAN");
	}else if(md.dirty == MD_DIRTY){
		printf("md.dirty\t%s\n","DIRTY");
		printf("md.io_node_head#%s\n",md.io_node_head);
		printf("md.io_node_tail#%s\n",md.io_node_tail);
		len = strlen(md.io_node_head);
		bzero(p,MAX_ION_PATH);
		strncpy(p,md.io_node_head,len);
		len = strlen(p);
		printf("****************************\n");
		while(1){
			bzero(&ion,ION_SZ);
			if(ion_get(p,&ion) != 0){
				fprintf(stderr,"#%s ion_get fail!\n",p);
				break;
			}
			dlen = ion.length;
			printf("ion.key#%s\n",p);
			prtion(&ion);
			if(dlen > 0){
				io_data = (char*)malloc(dlen+2);
				bzero(io_data,dlen+1);
				iod_get(p,io_data);
				printf("io_data #%s\n",io_data);
				free(io_data);
			}
			len = strlen(ion.next);
			if(len == 0){
				break;
			}
			bzero(p,MAX_ION_PATH);
			strncpy(p,ion.next,len);
		}
	}else{
		printf("file #%s unrecognized md.dirty!\n",path);
	}
	printf("--------------- file %s end -------------\n",path);
	return;
}
static void print_fnl_ion(void)
{
	file_node * fn;
	printf("----------------------- file_node_list mdion start -------------------------\n");
	for_each_fn_in_fnlist(fn){
		print_ion(fn->file_name);
	}
	printf("------------------------ file_node_list mdion end --------------------------\n");
    return;
}
#define REQ_PATH(req)	((req)->path)
#define REQ_IONK(req)	((req)->ion_key)
#define REQ_IOT(req)	((req)->io_type)
#define REQ_OFF(req)	((req)->offset)
#define REQ_LEN(req)	((req)->length)
#define REQ_TIME(req)	((req)->m_atime)
static int que_out_modify_md(q_out_req *req)
{
	/* to update :
	 * 1) stat_info in meta data
	 * 2) io_node list for write operation */
    int rt = 0,len;
    Meta_Data md;
    IO_Node ion,iontmp;
	if(md_get(REQ_PATH(req),&md) != 0){
		fprintf(stderr,"me_get fail,this file may have been deleted!\n");
		rt = 1;
		goto ret;
	}
	if(REQ_IOT(req) == IO_WRITE){
		/* make a new io_ndoe */
		bzero(&ion,ION_SZ);
		ion.modification_time = REQ_TIME(req);
		ion.offset = REQ_OFF(req);
		ion.length = REQ_LEN(req);
#ifdef DBGMSG1
		printf("before update md ion iontmp\n");
		prtion(&ion);
#endif
		/* add to io_node list */
		if(md.dirty == MD_CLEAN && strlen(md.io_node_head) == 0 && strlen(md.io_node_tail) == 0){
#ifdef DBGMSG
			printf("io_node list is null\n");
#endif
			bzero(md.io_node_head,MAX_ION_PATH);
			len = strlen(REQ_IONK(req));
			strncpy(md.io_node_head,REQ_IONK(req),len);
			bzero(md.io_node_tail,MAX_ION_PATH);
			strncpy(md.io_node_tail,REQ_IONK(req),len);
#ifdef DBGMSG1
			printf("md.io_node_head&tail are assigned with new value!\n");
			printf("md.io_node_head #%s\n",md.io_node_head);
			printf("md.io_node_tail #%s\n",md.io_node_tail);
#endif
		}else if(md.dirty == MD_DIRTY && strlen(md.io_node_head) != 0 && strlen(md.io_node_tail) != 0){
#ifdef DBGMSG
			printf("io_node list is not null,and it's dirty,head&tail not null\n");
#endif
			/* modify tail node */
			bzero(&iontmp,ION_SZ);
			if(ion_get(md.io_node_tail,&iontmp) != 0){
				rt = 2;
				goto ret;
			}
#ifdef DBGMSG1
			printf("current tail io node\n");
			prtion(&iontmp);
#endif
			len = strlen(REQ_IONK(req));
			bzero(iontmp.next,MAX_ION_PATH);
			strncpy(iontmp.next,REQ_IONK(req),len);
			if(ion_put(md.io_node_tail,&iontmp) != 0){
				rt = 3;
				goto ret;
			}
#ifdef DBGMSG1
			printf("tail node updated to :\n");
			prtion(&iontmp);
			printf("io_node_tail is put back!\n");
#endif
			/* modify ion */
			len = strlen(md.io_node_tail);
			strncpy(ion.pre,md.io_node_tail,len);
			/* modify tail pointer */
			len = strlen(REQ_IONK(req));
			bzero(md.io_node_tail,MAX_ION_PATH);
			strncpy(md.io_node_tail,REQ_IONK(req),len);
#ifdef DBGMSG
			printf("ion.pre is assigned to the previous io_node_tail,and new tail are assigned to new io_node\n");
#endif
		}else{
#ifdef DBGMSG
			printf("unaccepted io_node list state!\n");
#endif
			rt = 4;
			goto ret;
		}
		/* put new io node back */
		if(ion_put(md.io_node_tail,&ion) != 0){
			fprintf(stderr,"io node put fail!\n");
			rt = 5;
			goto ret;
		}
#ifdef DBGMSG1
		printf("ion right when it is put back!\n");
		prtion(&ion);
		printf("ion with key #%s put\n",md.io_node_tail);
#endif
		md.mtime = REQ_TIME(req);
		md.dirty = MD_DIRTY;
    }else if(REQ_IOT(req) == IO_READ){
		/* just modify atime */
		md.atime = REQ_TIME(req);
	}else{
#ifdef DBGMSG
		printf("unaccepted io_type!\n");
#endif
		rt = 6;
		goto ret;
	}
	if(md_put(REQ_PATH(req),&md) != 0){
		fprintf(stderr,"md_put fail!\n");
		rt = 7;
	}
#ifdef DBGMSG
	printf("md is put back with key # %s\n",REQ_PATH(req));
#endif
ret:
    return rt;
}
static void prtreq(q_out_req * req)
{
	printf("----------- req start ------------\n");
	printf("file to modify\t#%s\n",req->path);
	printf("io_node key\t#%s\n",req->ion_key);
	printf("io_time\t\t#%d\n",req->m_atime);
	printf("io_type\t\t#%s\n",(req->io_type==IO_READ?"READ":"WRITE"));
	printf("io_offset\t#%d\n",req->offset);
	printf("io_length\t#%d\n",req->length);
	printf("------------ req end -------------\n");
	return;
}
static void* que_out_serv(void*arg)
{
	/* 1) receive q_out request 
	 * 2) modify metadata(io_node list)
	 * 3) modify file node list */
    int port,listenfd,connfd,len,n;
    q_out_req req;
    char ip_str[INET_ADDRSTRLEN];
    struct sockaddr_in addr,cli_addr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET,SERV_IP,(void*)&addr.sin_addr);
    addr.sin_port = htons(Q_OUT_SERV_PORT);
    len = sizeof(addr);
    bind(listenfd,(struct sockaddr *)&addr,len);
    listen(listenfd,LISTENQ);
    for(;;){
        len = sizeof(cli_addr);
        connfd = accept(listenfd,(struct sockaddr *)&cli_addr,&len);
        bzero(ip_str,INET_ADDRSTRLEN);
        inet_ntop(AF_INET,(void*)&cli_addr.sin_addr,ip_str,INET_ADDRSTRLEN);
        port = ntohs(cli_addr.sin_port);
        printf("--------------- que_out request from ----# %s : %d\n",ip_str,port);

		bzero(&req,QOUT_REQ_SZ);
        if(read(connfd,&req,QOUT_REQ_SZ) != QOUT_REQ_SZ){
			perror("q_out server read");
            continue;
        }
		/* modify md in TC */
#ifdef DBGMSG
		printf("-------------- q_out received a new request ----------------\n");
		prtreq(&req);
#endif
#ifdef DBGMSG
		printf("before modify meta data and io_node list...\n");
		print_ion(req.path);
#endif
        pthread_mutex_lock(&md_mutex);
        if(que_out_modify_md(&req) != 0){
			fprintf(stderr,"modify md fail!\n");
		}
#ifdef DBGMSG
		printf("after modify meta data and io_node list...\n");
		print_ion(req.path);
#endif
        pthread_mutex_unlock(&md_mutex);

#ifdef DBGMSG
		printf("before modify file node list:\n");
		print_file_node_list();
#endif
		/* modify file node list in memory */
        pthread_mutex_lock(&file_node_list_mutex);
		add2file_node_list_head(req.path);
#ifdef DBGMSG
		printf("after modify file node list:\n");
		print_file_node_list();
#endif
        pthread_mutex_unlock(&file_node_list_mutex);
		/* tell flush thread */
		pthread_cond_signal(&file_node_list_cond);
#ifdef DBGMSG
		printf("q_out : send signal to flush thread...");
        printf("Waiting for next request...\n");
#endif
        close(connfd);
    }
    pthread_exit(NULL);
}
static void flush2data_center(void)
{
	/* update file according to the file node list */
	int fd,ion_len;
	void * io_data;
	Meta_Data md;
	IO_Node ion;
	file_node * fn;
	char data_path[MAX_PATH];
	char file_to_update[MAX_PATH];
//	/* go to sleep first for debug */
//	printf("flush go to sleep\n");
//	pause();
	while(1){
		/* 1) in this while loop,we first get a file_node from file_node_list,
		 *    which is always the node pointed to by 'update'.
		 * 2) Then we flush the dirty data of this file to the data center represented by this file_node.
		 * */
		pthread_mutex_lock(&file_node_list_mutex);
		while(FILE_NODE_LIST_NULL || (fn = update) == NULL){
			/* go to sleep if file node list is null or update is null */
#ifdef DBGMSG
			printf("file_node_list is NULL or 'update' == NULL,flush now go to sleep.\n");
#endif
			pthread_cond_wait(&file_node_list_cond,&file_node_list_mutex);
#ifdef DBGMSG
			printf("flush awakened! now retest the condition!\n");
#endif
		}
		/* update points to the prev node */
		update = prev_node_of_me(update);
		pthread_mutex_unlock(&file_node_list_mutex);
#ifdef DBGMSG
		printf("now flush file #%s to data center!\n",fn->file_name);
#endif
		bzero(file_to_update,MAX_PATH);
		strcpy(file_to_update,fn->file_name);
		pthread_mutex_lock(&md_mutex);
		if(md_get(fn->file_name,&md) != 0 || md.dirty == MD_CLEAN){
			/* skip this file for reasons below:
			 * 1) this file has been deleted
			 * 2) this file is clean
			 */
			goto next_loop;
		}
		if(!(strlen(md.io_node_head) > 0 && strlen(md.io_node_tail) > 0 && md.dirty == MD_DIRTY)){
			/* when the file is dirty,we expect this file 
			 *	1) io_node_head != NULL
			 *	2) io_node_tail != NULL
			 *	3) md.dirty == MD_DIRTY
			 *	*/
			fprintf(stderr,"something is wrong with the state of this file!\n");
			goto next_loop;
		}
		/* here we got the only accepted case to update the data center file */
		/* everything is just ok for updating file
		 * update file represented by 'fn' */
		get_data_path(fn->file_name,data_path);
		fd = 0;
		if((fd = open(data_path,O_WRONLY)) < 0){
			perror("open data center file");
			if(errno == ENOENT){
				/* deleted */
				fprintf(stderr,"file %s has been deleted!\n",fn->file_name);
			}
			goto next_loop;
		}
		ion_len = strlen(md.io_node_head);
		while(ion_len > 0){
#ifdef DBGMSG
			printf("flush : now get io_node with key #%s\n",md.io_node_head);
#endif
			if(ion_get(md.io_node_head,&ion) != 0){
				goto next_loop;
			}
#ifdef DBGMSG
			printf("ion_get success!\n");
			prtion(&ion);
			printf("md->mtime\t#%d\n",md.mtime);
#endif

			if(ion.length > 0){
				io_data = malloc(ion.length+2);
				bzero(io_data,ion.length+2);
				if(iod_get(md.io_node_head,io_data) != 0){
					fprintf(stderr,"io_data get fail,maybe this file has been deleted!\n");
					free(io_data);
					goto next_loop;
				}
#ifdef DBGMSG
				printf("io_data #%s\n",(char*)io_data);
#endif
				lseek(fd,ion.offset,SEEK_SET);
				if(write(fd,io_data,ion.length) != ion.length){
					perror("write to dtc file");
					free(io_data);
					goto next_loop;
				}
				free(io_data);
#ifdef DBGMSG
				printf("write data center file ok! now delete io_node and io_data\n");
#endif
				if(iod_out(md.io_node_head) != 0){
					goto next_loop;
				}
#ifdef DBGMSG
				printf("io_data deleted!\n");
#endif
			}
			if(ion_out(md.io_node_head) != 0){
				goto next_loop;
			}
#ifdef DBGMSG
			printf("io_node deleted!\n");
#endif
			ion_len = strlen(ion.next);
			if(ion_len == 0){
				break;
			}
			bzero(md.io_node_head,MAX_ION_PATH);
			strncpy(md.io_node_head,ion.next,ion_len);
		}
#ifdef DBGMSG
		printf("update file ok! # %s\nnow clear io_node_head&tail,mark this file to be clean!\n",fn->file_name);
#endif
		bzero(md.io_node_head,MAX_PATH);
		bzero(md.io_node_tail,MAX_PATH);
		md.dirty = MD_CLEAN;
		md_put(fn->file_name,&md);
#ifdef DBGMSG
		/* this file has been updated to data center 
		 * remove from file node list 
		 * But theoretically,we shouldn't remove file node here,
		 * because the node which has been updated will be pushed to the tail end of the list,
		 * they will be candidates to be swap out of cache when there is no enough space for new file */
		printf("remove current update file from file node list!");
#endif
		rm_fn_from_list(fn);
		delete_file_node(fn);
next_loop:
		if(fd>0){close(fd);}
		pthread_mutex_unlock(&md_mutex);
	}
	return;
}
int main()
{
    pthread_t tid_que_out;
#ifdef DBGMSG
	printf("initializing file node list ...\n");
#endif
	init_fn_list();
#ifdef DBGMSG
	printf("creat q_out thread ...\n");
#endif
    if(pthread_create(&tid_que_out,NULL,que_out_serv,NULL) != 0){
        perror("que_out_serv thread_create");
        exit(1);
    }
#ifdef DBGMSG
	printf("start flush2data_center routine ...\n");
#endif
	flush2data_center();
    pthread_join(tid_que_out,NULL);
	return 0;
}
