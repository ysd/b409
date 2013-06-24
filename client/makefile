CC = gcc -g
INCLUDE = /usr/local/include/libxml2
LIB = /usr/local/lib
CFLAGS = -lxml2 -lm -lz
CURLFLAGS=-lcurl

.PHONY:client parser_xml_file  s3api clean 

client:client.o command.o command_split.o parser_xml_file.o s3api.o
	$(CC) -I$(INCLUDE) -L$(LIB) $(CFLAGS) $(CURLFLAGS) -o client client.o command.o command_split.o parser_xml_file.o s3api.o
	
parser_xml_file:parser_xml_file.o
	$(CC) -I$(INCLUDE) -L$(LIB) $(CFLAGS) -o parser_xml_file parser_xml_file.o

s3api:s3api.o md5.o
	$(CC) -I$(INCLUDE) -L$(LIB) $(CURLFLAGS) -o s3api s3api.o md5.o


client.o:client.c  common.h command_split.h command.h s3api.h
	$(CC) -I$(INCLUDE)  -c client.c

command.o:command.c command.h common.h
	$(CC) -I$(INCLUDE)  -c command.c
command_split.o:command_split.c command_split.h common.h
	$(CC) -I$(INCLUDE)  -c command_split.c


parser_xml_file.o: parser_xml_file.c parser_xml_file.h common.h
	$(CC) -I$(INCLUDE) -c parser_xml_file.c 

md5.o:md5.c md5.h
	$(CC) -c md5.c
s3api.o:s3api.c md5.h s3api.h common.h
	$(CC) -I$(INCLUDE) -c  s3api.c




clean:
	rm client parser_xml_file s3api client.o command.o input.o parser_xml_file.o md5.o s3api.o
	


