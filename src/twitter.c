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

/* constants */
const char *request_token_uri = "https://api.twitter.com/oauth/request_token";
const char *access_token_uri  = "https://api.twitter.com/oauth/access_token";
const char *authorize_uri     = "https://api.twitter.com/oauth/authorize";
const char *req_c_key         = "yo9tIaQs7prILLMSq3DQiQ"; //< consumer key
const char *req_c_secret      = "EwOoyEkpb9STlZE6F0HtqofHhcPPhbpUpQel5lWoM"; //< consumer secret

/* global */
char *req_t_key    = NULL;
char *req_t_secret = NULL;


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

static char* curl_http_get(char *req_url) {
	char *reply;
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
	return reply;
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
    reply = curl_http_get(req_url); /* GET */
    
    if (!reply)
        printf("HTTP request for an oauth request-token failed.\n");
    else {
        printf("HTTP-reply: %s\n", reply);
    }

    if(req_url) free(req_url);
    if(reply) return reply;
    else      return NULL;
}

/* sub routines */
/*
 * a example requesting and parsing a request-token from an OAuth service-provider
 * excercising the oauth-HTTP GET function. - it is almost the same as
 * \ref request_token_example_post below.
 */
char* request_token_example_get(void) {
    char *req_url = NULL;
    char *reply;
	char *request_oauth_url = (char*)malloc(MAXSTRINGLENGTH);
    memset(request_oauth_url, 0, MAXSTRINGLENGTH);
    req_url = oauth_sign_url2(request_token_uri, NULL, OA_HMAC,
                              NULL, conf->CONSUMER_KEY, conf->CONSUMER_SECRET, NULL, NULL);

    printf("request URL:%s\n\n", req_url);
    reply = curl_http_get(req_url); /* GET */
    if (!reply)
        printf("HTTP request for an oauth request-token failed.\n");
    else {
        int rc;
        char **rv = NULL;

        printf("HTTP-reply: %s\n", reply);
        rc = oauth_split_url_parameters(reply, &rv);
        qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
        printf("rc=%d rv[0]=%s rv[1]=%s\n", rc, rv[0], rv[1]);
        if(rc>=2) {
            int i;
            for(i=0; i<rc; i++) {
                if(!strncmp(rv[i], "oauth_token=", 12))
                    req_t_key=strdup(&(rv[i][12]));
                if(!strncmp(rv[i], "oauth_token_secret=", 19))
                    req_t_secret=strdup(&(rv[i][19]));
            }
            printf("key:    %s\nsecret: %s\n",req_t_key, req_t_secret);
            printf("auth:   %s?oauth_token=%s\n", authorize_uri, req_t_key);
			snprintf(request_oauth_url, MAXSTRINGLENGTH, "%s?oauth_token=%s", authorize_uri, req_t_key);
        }
        if(rv) free(rv);
    }

    if(req_url) free(req_url);
    if(reply) free(reply);
	return request_oauth_url;
}

bool access_token_example_get(char* pin) {
    char *res_t_key    = NULL; //< replied key
    char *res_t_secret = NULL; //< replied secret
    char *screen_name = NULL; 
    char verifier[1024];

    char *req_url = NULL;
    char *reply;
    char *postarg = NULL;
	bool ret = false;

    sprintf(verifier, "%s?&oauth_verifier=%s", access_token_uri, pin);
    printf("verifier=%s\n", verifier);
    req_url = oauth_sign_url2(verifier, NULL, OA_HMAC, NULL,
                              conf->CONSUMER_KEY, conf->CONSUMER_SECRET,
                              req_t_key, req_t_secret);

    printf("request URL:%s\n\n", req_url);

    reply = curl_http_get(req_url); /* GET */

    if (!reply) {
		printf("HTTP request for an oauth access-token failed.\n");
		return false;
	}
        
    else {
        int rc;
        char **rv = NULL;

        printf("HTTP-reply: %s\n", reply);

        rc = oauth_split_url_parameters(reply, &rv);
        qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
        printf("rc=%d rv[0]=%s rv[1]=%s\n", rc, rv[0], rv[1]);
        if(rc>=2) {
            int i;
            for(i=0; i<rc; i++) {
                if(!strncmp(rv[i], "oauth_token=", 12))
                    res_t_key=strdup(&(rv[i][12]));
                if(!strncmp(rv[i], "oauth_token_secret=", 19))
                    res_t_secret=strdup(&(rv[i][19]));
                if(!strncmp(rv[i], "screen_name=", 12))
                    screen_name=strdup(&(rv[i][12]));
            }
            printf("key:    '%s'\nsecret: '%s'\nscreen_name: '%s'\n", res_t_key, res_t_secret,screen_name);
			if(conf->TwitterID) free(conf->TwitterID);
			if(conf->OAUTH_TOKEN) free(conf->OAUTH_TOKEN);
			if(conf->OAUTH_TOKEN_SECRET) free(conf->OAUTH_TOKEN_SECRET);
			conf->TwitterID = screen_name;
			conf->OAUTH_TOKEN = res_t_key;
			conf->OAUTH_TOKEN_SECRET = res_t_secret;
			ret = true;
        }

        free(rv);
    }

    if(req_url) free(req_url);
    if(reply) free(reply);
    if(res_t_key) free(res_t_key);
    if(res_t_secret) free(res_t_secret);
    if(screen_name) free(screen_name);
	return ret;
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
