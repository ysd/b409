link_lib = -L/usr/local/lib -ltokyocabinet -lz -lbz2 -lrt -lpthread -lm -lc
objects = md_func.o utility.o posix_api.o
obj_main = _creat.o _read.o _write.o _remove.o super_serv_process.o 
.PHONY : all _creat _read _write _remove super_serv_process 
all : _creat _read _write _remove super_serv_process 
_creat : _creat.o $(objects)
	gcc _creat.o $(objects) -g -o _creat $(link_lib)
_read : _read.o $(objects)
	gcc _read.o $(objects) -g -o _read $(link_lib)
_write : _write.o $(objects)
	gcc _write.o $(objects) -g -o _write $(link_lib)
_remove : _remove.o $(objects)
	gcc _remove.o $(objects) -g -o _remove $(link_lib)
super_serv_process : super_serv_process.o $(objects)
	gcc super_serv_process.o $(objects) -g -o super_serv_process $(link_lib) 
.PHONY : clean
clean :
	rm _creat _read _write _remove super_serv_process $(objects) $(obj_main) 
