LIBS = -L/usr/local/lib -ltokyocabinet -lz -lbz2 -lrt -lpthread -lm -lc -lxml2 -lhiredis
XML_INCL = -I/usr/include/libxml2
OBJS = md_func.o utility.o nss.o md5.o name_buf.o xml.o bloom.o hash.o blk_idx.o namespace_server.o dedup_test.o redis.o
BINS = namespace dedup_test redis
.PHONY : all namespace dedup_test redis
all : namespace dedup_test redis
xml.o : 
	gcc $(XML_INCL) -c xml.c
namespace_server.o :
	gcc $(XML_INCL) -c namespace_server.c
namespace : namespace_server.o utility.o md_func.o md5.o name_buf.o xml.o nss.o
	gcc namespace_server.o utility.o md_func.o md5.o name_buf.o xml.o nss.c -o namespace $(LIBS)
dedup_test : dedup_test.o md5.o hash.o bloom.o blk_idx.o
	gcc dedup_test.o md5.o hash.o bloom.o blk_idx.o -o dedup_test
redis : redis.o
	gcc redis.o -o redis $(LIBS)
.PHONY : clean
clean :
	rm $(BINS) $(OBJS) $(obj_main) 
