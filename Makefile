#makefile
CC_OP = -g -o
CFLAGS = -I/usr/local/include/libxml2 -I/usr/local/include
LINK_LIB = -L/usr/local/lib  -ltokyocabinet -lbz2 -lresolv -lnsl -lc -ldl -lrt -lxml2 -lz -lm -lpthread -lmicrohttpd  -ltokyotyrant
OBJS= bucket_response_header.o s3_server.o delete_bucket.o errmsg.o md5.o md_func.o name_buf.o nss.o nss_msg.o object_response_header.o posix_api.o request_analysis.o utility.o xml.o xml_s3.o
LOGIN_OBJS=  login_server.o command.o authen.o command_split.o  md5.o md_func.o nss.o nss_msg.o xml.o utility.o  name_buf.o
NSS_OBJS= nss_server.o command.o authen.o command_split.o md5.o md_func.o nss.o nss_msg.o xml.o utility.o name_buf.o 

ALL = s3_server login_server nss_server
.PHONY :all login_server
all : s3_server login_server nss_server
s3_server : $(OBJS)
	gcc $(CFLAGS) $(OBJS) $(CC_OP) s3_server $(LINK_LIB)
login_server : $(LOGIN_OBJS)
	gcc $(CFLAGS) $(LOGIN_OBJS) $(CC_OP) login_server $(LINK_LIB)
nss_server : $(NSS_OBJS)
	gcc $(CFLAGS) $(NSS_OBJS) $(CC_OP) nss_server $(LINK_LIB)
.PHONY : clean
clean : 
	rm *.o $(ALL)
