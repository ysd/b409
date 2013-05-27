#include"global.h"
#include"md_type.h"
#include"utility.h"
static int init_file(char* path)
{
	/* initialize meta data for this file */
	int rt = 0;
	char data_path[MAX_PATH];
	struct stat my_stat;
	Meta_Data md;
	bzero(&md,MD_SZ);
	get_data_path(path,data_path);
	if(stat(data_path,&my_stat) != 0){
		perror("stat");
		rt = 1;
		goto ret;
	}
	memcpy((void*)&(md.stat_info),(void*)&my_stat,sizeof(struct stat));
	md.access_time_queue.head = 0;
	md.access_time_queue.tail = 0;
	md.dirty = MD_CLEAN;
	bzero(md.io_node_head,MAX_PATH);
	bzero(md.io_node_tail,MAX_PATH);
	if(md_put(path,&md) != 0){
		fprintf(stderr,"init file : md_put fail!\n");
		rt = 2;
	}
ret:
	return rt;
}
static int de_init_file(char *path)
{
	/* called when delete a file 
	 * 1) delete all the io_nodes and io_data of this file 
	 * 2) delete meta data of this file 
	 * 3) as for file_node,just leave it alone,
	 * so every time before updating file according file node list,
	 * check if this file still exist. 
	 * So here no strong consitency is ensured,just handle it. */
	int rt = 0;
	int len;
	char ionp[MAX_ION_PATH];
	Meta_Data md;
	IO_Node ion;
	if(md_get(path,&md) != 0){
		rt = 1;
		goto ret;
	}
	if(md.dirty == MD_CLEAN){
		goto del_md;
	}
	len = strlen(md.io_node_head);
	strncpy(ionp,md.io_node_head,len);
	while(len != 0){
		/* delete io_node and io_data */
		ion_get(ionp,&ion);
		ion_out(ionp);
		iod_out(ionp);
		len = strlen(ion.next);
		strncpy(ionp,ion.next,len);
	}
	/* delete meta data */
del_md:
	md_out(path);
ret:
	return rt;
}
static int que_out(const q_out_req *req)
{
	/* send request to super process */
	int rt = 0,cskt,c_len;
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET,SERV_IP,(void *)&addr.sin_addr);
	addr.sin_port = htons(Q_OUT_SERV_PORT);
	c_len = sizeof(addr);
	if((cskt = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Socket");
		rt = 1;
		goto ret;
	}
	if(connect(cskt,(struct sockaddr*)&addr,c_len) == -1){
		perror("Connection");
		rt = 2;
		goto over;
	}
	if(write(cskt,req,QOUT_REQ_SZ) != QOUT_REQ_SZ){
		perror("write socket");
		rt = 3;
	}
over:
	close(cskt);
ret:
	return rt;
}
int _CreateFile(char* path,u32 perm,int mode)
{   
	/* 1) create cache file
	 * 2) create data_center file
	 * 3) init_file
	 * 4) open cache file with 'mode' 
	 * */
	int fd;
	char dst_path[MAX_PATH];
	get_cache_path(path,dst_path);
	if((fd = creat(dst_path,perm)) == -1){
		perror("create cache file");
		return fd;
	}
	close(fd);
	get_data_path(path,dst_path);
	if((fd = creat(dst_path,perm)) == -1){
		perror("create data center file");
		return fd;
	}
	close(fd);
	/* init file */
	if(init_file(path) != 0){
		fprintf(stderr,"init_file fail!\n");
		return -1;
	}
	fd = open(dst_path,mode);
	return fd;
}
int _OpenFile(char* path,int mode)
{
	/* 1) open cache first,if ok,return opened fd 
	 * 2) if cache file open fail,open dtc file instead*/
	int fd;
	char dst_path[MAX_PATH];
	get_cache_path(path,dst_path);
	if((fd = open(dst_path,mode)) == -1){
		/* open dtc file */
		perror("Open cache_file");
		get_data_path(path,dst_path);
		if((fd = open(dst_path,mode)) == -1){
			perror("Open dtc_file");
		}
	}
	return fd;
}
int _CloseFile(int fd)
{
	return close(fd);
}
int _Remove(char* path)
{
	/* 1) remove cache file 
	 * 2) remove dtc file 
	 * 3) remove md */
	int rt = 0;
	char dst_path[MAX_PATH];
	get_cache_path(path,dst_path);
	if((rt = remove(dst_path)) != 0){
		perror("Remove cache file");
		goto ret;
	}
	get_data_path(path,dst_path);
	if((rt = remove(dst_path)) != 0){
		perror("Remove data center file");
		goto ret;
	}
	if(de_init_file(path) != 0){
		rt = 1;
	}
ret:
	return rt;
}
int _ReadFile(int fd,u8 * buf,u32 count,u64 offset)
{
	/* when read finishes,update the file_node list */
	int rt = 0,io_c_d = -1;
	char cd_path[MAX_PATH],* p;
	q_out_req req;
	time_t iotime;
	get_path_from_fd(fd,cd_path);
	if(strncmp(cd_path,CACHE_PATH,strlen(CACHE_PATH)) == 0){
		p = cd_path + strlen(CACHE_PATH);
		io_c_d = IO_CACHE;
		//que_in(path);
	}else if(strncmp(cd_path,DATA_PATH,strlen(DATA_PATH)) == 0){
		p = cd_path + strlen(DATA_PATH);
		io_c_d = IO_DATA;
	}else{
		fprintf(stderr,"unrecognized path!\n");
		return -1;
	}
	while(*p == '/'){p++;}	/* eat '/' */
	lseek(fd,offset,SEEK_SET);
	rt = read(fd,buf,count);
	time(&iotime);
	if(io_c_d == IO_CACHE){
		/* tell super_process to update file node list 
		 * no io_data need to be recorded */
		bzero(&req,QOUT_REQ_SZ);
		strncpy(req.path,p,strlen(p));
		req.m_atime = iotime;
		req.io_type = IO_READ;
		req.offset = offset;
		req.length = count;
		if(que_out(&req) != 0){
			fprintf(stderr,"q_out fail!\n");
			return -1;
		}
	}
	return rt;
}
int _WriteFile(int fd,u8* buf,u32 count,u64 offset)
{
	int rt = 0;
	int io_c_d = -1;
	char cd_path[MAX_PATH],* p;
	q_out_req req;
	time_t mt;
	pid_t pid = getpid();
	bzero(&req,sizeof(q_out_req));
	get_path_from_fd(fd,cd_path);
	printf("_WriteFile:get path from fd -- %s\n",cd_path);
	if(strncmp(cd_path,CACHE_PATH,strlen(CACHE_PATH)) == 0){
		p = cd_path + strlen(CACHE_PATH);
		io_c_d = IO_CACHE;
		//que_in(path);
	}else{
		p = cd_path + strlen(DATA_PATH);
		io_c_d = IO_DATA;
	}
	while(*p == '/'){p++;}	/* eat '/' */
	lseek(fd,offset,SEEK_SET);
	rt = write(fd,buf,count);
	time(&mt);
	if(io_c_d == IO_CACHE){
		bzero(&req,QOUT_REQ_SZ);
		strncpy(req.path,p,strlen(p));
		req.m_atime = mt;
		req.io_type = IO_WRITE;
		req.offset = offset;
		req.length = count;
		/* generate key for io_data & io_node 
		 * FORMAT : pid_time_filename 
		 * */
		bzero(req.ion_key,MAX_ION_PATH);
		snprintf(req.ion_key,MAX_ION_PATH,"%d_%d_%s",pid,mt,p);
		/* put io_data into TC */
		if(iod_put(req.ion_key,buf,count) != 0){
			fprintf(stderr,"iod_put fail!\n");
			return -1;
		}
		if(que_out(&req) != 0){
			fprintf(stderr,"q_out fail!\n");
			return -1;
		}
	}
	return rt;
}
