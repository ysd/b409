#ifndef _IO_API_H
#define _IO_API_H
#include"global.h"
extern int _creat(char * file_name);
extern int _read(char * file_name,u8 * buf,u32 count,u64 offset);
extern int _write(char * file_name,u8 * buf,u32 count,u64 offset);
extern int _rm(char * file_name);
#endif
