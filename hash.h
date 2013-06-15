#ifndef _HASH_H
#define _HASH_H
#include"global.h"


/* hashfunc_t is pointer to the hash function */
typedef u32 (*hashfunc_t)(char*);
#define HASH_FUNC_T_SZ sizeof(hashfunc_t)

/* A Simple Hash Function */
extern u32 simple_hash(char *str);

/* RS Hash Function */
extern u32 RS_hash(char *str);

/* JS Hash Function */
extern u32 JS_hash(char *str);

/* P. J. Weinberger Hash Function */
extern u32 PJW_hash(char *str);

/* ELF Hash Function */
extern u32 ELF_hash(char *str);

/* BKDR Hash Function */
extern u32 BKDR_hash(char *str);

/* SDBM Hash Function */
extern u32 SDBM_hash(char *str);

/* DJB Hash Function */
extern u32 DJB_hash(char *str);

/* AP Hash Function */
extern u32 AP_hash(char *str);

/* CRC Hash Function */
extern u32 CRC_hash(char *str);

#endif
