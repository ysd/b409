#include"global.h"
_PROTOTYPE(int _CreateFile,(char* path,u32 perm,int mode));
_PROTOTYPE(int _OpenFile,(char* path,int mode));
_PROTOTYPE(int _CloseFile,(int fd));
_PROTOTYPE(int _Remove,(char* path));
_PROTOTYPE(int _ReadFile,(int fid,u8 *buf,u32 count,u64 offset));
_PROTOTYPE(int _WriteFile,(int fd,u8 *buf,u32 count,u64 offset));
