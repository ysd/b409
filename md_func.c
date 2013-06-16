#include"global.h"
#include"md_type.h"
static inline int get_db(tc_t type,char **db)
{
	switch(type){
		case MD:
			*db = META_DATA_DB;
			return MD_SZ;
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
		fprintf(stderr,"(key=%s) md_get error:%s\n",key,tchdberrmsg(ecode));
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
		fprintf(stderr,"(key=%s) tc_out error:%s\n",key,tchdberrmsg(ecode));
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
		fprintf(stderr,"(key=%s) tc_put error:%s\n",key,tchdberrmsg(ecode));
		rt = 2;
	}
ret:
	tchdbdel(hdb);
	return rt;
}
int md_get(char *path,meta_data_t * md)
{
	return tc_get((void*)path,(void*)md,MD);
}
int md_out(char * path)
{
	return tc_out((void*)path,MD);
}
int md_put(char * path,meta_data_t * meta_data)
{
	return tc_put((void*)path,(void*)meta_data,0,MD);
}
/* initialize the meta data for object 
 * @md5s : md5 of object full path from namespce */
int init_md_of_obj(char * md5s)
{
	meta_data_t md;
	time_t t = time(NULL);
	bzero(&md,MD_SZ);
	md.atime = t;
	md.ctime = t;
	md.mtime = t;
	md.size = 0;
	/* originally not in cache */
	clear_cache_bit(&md);
	strcpy(md.replica[0].rep_ip,"192.168.0.244");
	return md_put(md5s,&md); 
}
int de_init_md_of_obj(char *md5s)
{
	return md_out(md5s);
}
