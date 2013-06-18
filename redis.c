#include"global.h"
char * lmsg = "lrange msg 0 4";
char * sm = "smembers ms";
char * _w = "watch cc";
char * _g = "get cc";
char * _m = "multi";
char * _p = "set cc %d";
char * _e = "exec";
char * ism = "sismember ms %d";
static void prt_rpl(redisReply * rpl)
{
	redisReply * rple;
	printf("redis reply type #%s\n",redis_reply_type_str(rpl->type));
	printf("redis reply integer #%lld\n",rpl->integer);
	printf("redis reply len #%d\n",rpl->len);
	printf("redis reply str #%s\n",rpl->str);
	printf("redis reply elements #%d\n",rpl->elements);
}
int main()
{
	int val=0,i=0;
	redisContext *c = redisConnect(REDIS_IP,REDIS_PORT);
	if( c != NULL && c->err) {
		fprintf(stderr,"error %s\n",c->errstr);
		return 1;
	}
	redisReply * rpl;
	rpl = redisCommand(c,ism,1);
	prt_rpl(rpl);
	rpl = redisCommand(c,ism,2);
	prt_rpl(rpl);
	rpl = redisCommand(c,ism,3);
	prt_rpl(rpl);
	rpl = redisCommand(c,ism,4);
	prt_rpl(rpl);
//	rpl = redisCommand(c,"SET foo %s",ism);
//	prt_rpl(rpl);
//	rpl = redisCommand(c,"GET ww");
//	prt_rpl(rpl);
//	rpl = redisCommand(c,lmsg);
//	prt_rpl(rpl);
//	rpl = redisCommand(c,sm);
//	prt_rpl(rpl);

//	/* watch */
//	printf("/* watch */\n");
//	rpl = redisCommand(c,_w);
//	prt_rpl(rpl);
//
//	/* get */
//	printf("/* get */\n");
//	rpl = redisCommand(c,_g);
//	prt_rpl(rpl);
//
//	while(i<rpl->len){
//		val = val*10 + *(rpl->str + (i++)) - '0';
//	}
//
//	printf("val == %d\n",val);
//	/* modify */
//	printf("/* modify */\n");
//	val += 1;
//	printf("val == %d\n",val);
//
//	/* multi */
//	printf("/* multi */\n");
//	rpl = redisCommand(c,_m);
//	prt_rpl(rpl);
//
//	/* set */
//	printf("/* set */\n");
//	rpl = redisCommand(c,_p,val);
//	prt_rpl(rpl);
//
//	/* exec */
//	printf("/* exec */\n");
//	rpl = redisCommand(c,_e);
//	prt_rpl(rpl);
//
	redisFree(c);
	return 0;
}
