#ifndef _PARSER_XML_FILE_H_
#define _PARSER_XML_FILE_H_



extern int getHashAndIPFromCacheFile(char *cacheFileName,char *fileName,char *hash,char *ip);

extern int doBucketExists(char *BucketName);
extern int doObjectExistsInBucket(char *ObjectName,char *BucketName);
//缓存文件
extern int parserToCacheFile(char *cacheFileName,char *xmlFileName);

//list objects in bucket
extern int listObjectsInBucket(char *xmlFileName,char *BucketName);

//list buckets
extern int listBuckets(char *xmlFileName);

//list buckets and objects
extern int listAll(char *xmlFileName);

#endif
