#include "common.h"
#include "md5.h"
#include "s3api.h"
/*
*gcc -I/usr/local/include/libxml2  -L/usr/local/lib -lcurl -o s3api s3api.c md5.c
*/

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    printf("in write_data the  size is %d:\n",written);
    return written;
}

int S3GetObject(const char *local_filepath,const char * get_url)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char *url =(char*)malloc(strlen(get_url)+1);

    strncpy(url,get_url,strlen(get_url)+1);

    //	printf("url copy success!\n");
    puts(url);
    printf("the length of get_url is %d \n",strlen(get_url));
    printf("url  is   %s    &&   get_url is %s \n",url,get_url );
    char *outfilename=(char*)malloc(strlen(local_filepath)+1);
    strncpy(outfilename,local_filepath,strlen(local_filepath)+1);
    puts(local_filepath);

    //char outfilename[FILENAME_MAX] = "bxhdhgdf.txt";
//	char *outfilename=local_filepath;
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(outfilename,"wb");
        if (fp==NULL)
            printf("fopen error\n");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
        printf("get the file succeed\n");
        return 1;
    }
    else return 0;
}

int S3PutObject(const char *local_filepath,const char *put_url,const char* filename)
{
    CURL *curl;
    CURLcode res;

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);

    /* Fill in the file upload field */
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "sendfile",
                 // CURLFORM_FILE, "postit2.c",
                 CURLFORM_FILE, local_filepath,
                 CURLFORM_END);
    curl = curl_easy_init();
    /* initalize custom header list (stating that Expect: 100-continue is not
       wanted */
    headerlist = curl_slist_append(headerlist, buf);
    if (curl)
    {
        /* what URL that receives this POST */
        //curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.244:8888/r.c");
       // printf("before  reolve host!\n");
        curl_easy_setopt(curl, CURLOPT_URL, put_url);
       // printf("after  reolve host!\n");
       // printf("heh\n");
        // if ( (argc == 2) && (!strcmp(argv[1], "noexpectheader")) )
        /* only disable 100-continue header if explicitly requested */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */
        curl_formfree(formpost);
        /* free slist */
        curl_slist_free_all (headerlist);
        return 1;
    }
    return 0;
}

/*
void main()
{
        char file[]="/usr/local/tmp/m.txt";
        char url[]="/u5/b3/o8";
        unsigned char md5_checksum[16] = {0};
	unsigned char md5_str[33] = {0};
	md5(url,strlen(url), md5_checksum);
	md5_2_str(md5_checksum, md5_str);
	printf("md5: %s\n",md5_str);

        char buf[500];
	//上传
        snprintf(buf,sizeof(buf),"%s%s","http://192.168.0.244:8888",url);
	printf("buf:%s\n",buf);
 	S3PutObject(file,buf,file);

	//下载
      // snprintf(buf,sizeof(buf),"%s/%s","http://192.168.0.244:8888",md5_str);
       //S3GetObject("/usr/local/b.txt",buf);
}*/
