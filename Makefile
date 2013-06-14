#makefile
CC_OP = -g -o
CFLAGS = -I/usr/local/include/libxml2 -I/usr/local/include
LINK_LIB = -L/usr/local/lib  -ltokyocabinet -lbz2 -lresolv -lnsl -lc -ldl -lrt -lxml2 -lz -lm -lpthread -lmicrohttpd
OBJS= bucket_response_header.o container.o delete_bucket.o errmsg.o md5.o md_func.o name_buf.o nss.o  object_response_header.o posix_api.o request_analysis.o utility.o xml.o xml_s3.o

ALL = s3
.PHONY :all
all : s3
s3 : $(OBJS)
	gcc $(CFLAGS) $(OBJS) $(CC_OP) s3 $(LINK_LIB)

.PHONY : clean
clean : 
	rm $(OBJS_M) $(OBJS) $(ALL)
