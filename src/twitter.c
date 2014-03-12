/*
 * Main Test and Example Code.
 *
 * compile:
 *  gcc -lssl -loauth -lcurl -o twitter twitter.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "oauth.h"
#include "xmalloc.h"
#include "jconf.h"
#include "twittrouter.h"

struct MemoryStruct {
  char *data;
  size_t size; //< bytes remaining (r), bytes accumulated (w)

  size_t start_size; //< only used with ..AndCall()
  void (*callback)(void*,int,size_t,size_t); //< only used with ..AndCall()
  void *callback_data; //< only used with ..AndCall()
};

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->data = (char *)xrealloc(mem->data, mem->size + realsize + 1);
  if (mem->data) {
    memcpy(&(mem->data[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
  }
  return realsize;
}

/* sub routines */
/*
 * a example requesting and parsing a request-token from an OAuth service-provider
 * excercising the oauth-HTTP GET function. - it is almost the same as
 * \ref request_token_example_post below.
 */
static char* access_token_request_data(char *username) {
    char *req_url = NULL;
    char *reply;
    char friendship_url[MAXSTRINGLENGTH] = {'\0',};
    snprintf(friendship_url, MAXSTRINGLENGTH, "https://api.twitter.com/1.1/friendships/lookup.json?screen_name=%s,%s", conf->TwitterID, username);
    req_url = oauth_sign_url2(friendship_url, NULL, OA_HMAC,
                              NULL, conf->CONSUMER_KEY , conf->CONSUMER_SECRET ,
                              conf->OAUTH_TOKEN , conf->OAUTH_TOKEN_SECRET);

    printf("request URL:%s\n\n", req_url);
 //   reply = oauth_http_get(req_url,NULL); /* GET */
    CURL *curl;
    CURLcode res;
    //char *t1=NULL;
    struct MemoryStruct chunk;

    chunk.data=NULL;
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req_url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);


        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);


        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                               curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
        reply = chunk.data;

    }
    curl_global_cleanup();
    if (!reply)
        printf("HTTP request for an oauth request-token failed.\n");
    else {
        printf("HTTP-reply: %s\n", reply);
    }

    if(req_url) free(req_url);
    if(reply) return reply;
    else      return NULL;
}

static bool parser_friendship_json(char *friendship) {
    if (!friendship) return false;
    char *first_front = strchr(friendship,'{');
    char *second_front = strrchr(friendship,'{');

    if(first_front && first_front != second_front ) {
        char *connections = strstr(second_front,"connections");
        if(connections == NULL) {
            return false;    
        }
        char *colon = strchr(connections,':');
        if(strncmp(strchr(colon,'"'),"\"none",5) == 0)
            return false;
        else
            return true;
    }else {
        return false;
    }
}

bool get_friendship(char *username) {
    char *friendship = access_token_request_data(username);
    if(friendship != NULL && parser_friendship_json(friendship) == true) {
        free(friendship);
        return true;
    } else if(friendship != NULL) {
        free(friendship);
    }
    return false;
}
