link_lib = -L/usr/local/lib -ltokyocabinet -lz -lbz2 -lrt -lpthread -lm -lc -lxml2
xml_include = -I/usr/include/libxml2
objects = md_func.o utility.o nss.o md5.o name_buf.o xml.o
obj_main = namespace_server.o
all_bin = namespace
.PHONY : all namespace
all : namespace
xml.o : 
	gcc $(xml_include) -c xml.c
namespace_server.o :
	gcc $(xml_include) -c namespace_server.c
namespace : namespace_server.o utility.o md_func.o md5.o name_buf.o xml.o nss.o
	gcc namespace_server.o utility.o md_func.o md5.o name_buf.o xml.o nss.c -o namespace $(link_lib)
.PHONY : clean
clean :
	rm $(all_bin) $(objects) $(obj_main) 
