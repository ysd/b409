#ifndef _MEM_ALLOC
#define _MEM_ALLOC
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdint.h>
#define bzero(p,n)          memset(p,0,n)
/* type definition */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u64 Align;/* for allignment */
union header{
	struct{
		union header * nfp;/* next free block ptr */
		u32 size;
	}s;
	Align x;/* not used,just for alignment */
};
typedef union header Header;
#define nfp(h)		((h)->s.nfp)
#define szoffb(h)	((h)->s.size)
#define HEADER_SZ	sizeof(Header)
#define MINALLOC	1024
#endif
