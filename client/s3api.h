#ifndef _S3API_H_
#define _S3API_H_

extern size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

extern int S3GetObject(const char *local_filepath,const char * get_url);
extern int S3PutObject(const char *local_filepath,const char *put_url,const char* filename);

#endif
