link_lib = -L/usr/local/lib -ltokyocabinet -lz -lbz2 -lrt -lpthread -lm -lc -lxml2
xml_include = -I/usr/include/libxml2
objects = md_func.o utility.o nss.o md5.o name_buf.o xml.o bloom.o hash.o blk_idx.o
obj_main = namespace_server.o dedup_test.o
all_bin = namespace dedup_test
.PHONY : all namespace dedup_test
all : namespace dedup_test
xml.o : 
	gcc $(xml_include) -c xml.c
namespace_server.o :
	gcc $(xml_include) -c namespace_server.c
namespace : namespace_server.o utility.o md_func.o md5.o name_buf.o xml.o nss.o
	gcc namespace_server.o utility.o md_func.o md5.o name_buf.o xml.o nss.c -o namespace $(link_lib)
dedup_test : dedup_test.o md5.o hash.o bloom.o blk_idx.o
	gcc dedup_test.o md5.o hash.o bloom.o blk_idx.o -o dedup_test
.PHONY : clean
clean :
	rm $(all_bin) $(objects) $(obj_main) 
