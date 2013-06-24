#include"global.h"
#include"md_type.h"
static inline int get_db(tc_t type,char **db)
{
	switch(type){
		case MD:
			*db = META_DATA_DB;
			return MD_SZ;
		case ION:
			*db = IO_NODE_DB;
			return ION_SZ;
		case IOD:
			*db = IO_DATA_DB;
			return 0;
		default:
			fprintf(stderr,"unrecognized tc type!\n");
			break;
	}
	return 0;
}
static int tc_get(void * key,void * value,tc_t type)
{
	TCHDB *hdb;
	char * db;
	void * v;
	int ecode;
	int ksz = strlen((char*)key);
	int vsz = get_db(type,&db);
	int rt = 0;
	hdb = tchdbnew();
	if(!tchdbopen(hdb,db,HDBOREADER | HDBOCREAT)){
		ecode = tchdbecode(hdb);
		fprintf(stderr,"tc_get:open %s error -- %s\n",db,tchdberrmsg(ecode));
		rt = 1;
		goto ret;
	}
	v = tchdbget(hdb,key,ksz,&vsz);
	if(!value){
		ecode = tchdbecode(hdb);
		fprintf(stderr,"(key=%s) md_get error:%s\n",(char*)key,tchdberrmsg(ecode));
		rt = 2;
		goto ret;
	}
	memcpy(value,v,vsz);
	free(v);
ret:
	tchdbdel(hdb);
	return rt;
}
static int tc_out(void * key,tc_t type)
{
    TCHDB *hdb;
	int ecode;
	int rt = 0;
	int ksz = strlen((char*)key);
	char * db;
	get_db(type,&db);
	hdb = tchdbnew();
	if(!tchdbopen(hdb,db,HDBOWRITER))
	{
		ecode = tchdbecode(hdb);
		fprintf(stderr,"tc_out:open %s error -- %s\n",db,tchdberrmsg(ecode));
		rt = 1;
		goto ret;
	}
	if(!tchdbout(hdb,key,ksz)){
		ecode = tchdbecode(hdb);
		fprintf(stderr,"(key=%s) tc_out error:%s\n",(char *)key,tchdberrmsg(ecode));
		rt = 2;
	}
ret:
	tchdbdel(hdb);
	return rt;
}
static int tc_put(void * key,void * value,int len,tc_t type)
{
	TCHDB *hdb;
	char *db;
	int ecode;
	int rt = 0;
	int ksz = strlen((char*)key);
	int vsz = get_db(type,&db);
	if(type == IOD){
		vsz = len;
	}
	hdb = tchdbnew();
	if(!tchdbopen(hdb,db,HDBOWRITER | HDBOCREAT)){
		ecode = tchdbecode(hdb);
		fprintf(stderr,"tc_put:open %s error -- %s\n",db,tchdberrmsg(ecode));
		rt = 1;
		goto ret;
	}
	if(!tchdbput(hdb,key,ksz,value,vsz)){
		ecode = tchdbecode(hdb);
		fprintf(stderr,"(key=%s) tc_put error:%s\n",(char *)key,tchdberrmsg(ecode));
		rt = 2;
	}
ret:
	tchdbdel(hdb);
	return rt;
}
int md_get(char *path,Meta_Data * md)
{
	return tc_get((void*)path,(void*)md,MD);
}
int md_out(char * path)
{
	return tc_out((void*)path,MD);
}
int md_put(char * path,Meta_Data * meta_data)
{
	return tc_put((void*)path,(void*)meta_data,0,MD);
}
int ion_get(char *path_time,IO_Node *ion)
{
	return tc_get((void*)path_time,(void*)ion,ION);
}
int ion_out(char *path_time)
{
	return tc_out((void*)path_time,ION);
}
int ion_put(char *path_time,IO_Node *io_node)
{
	return tc_put((void*)path_time,(void*)io_node,0,ION);
}
int iod_get(char *path_time,void *iod)
{
	return tc_get((void*)path_time,iod,IOD);
}
int iod_out(char *path_time)
{
	return tc_out((void*)path_time,IOD);
}
int iod_put(char *path_time,void *iod,int len)
{
	return tc_put((void*)path_time,iod,len,IOD);
}
