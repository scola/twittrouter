/*
 * Main Test and Example Code.
 *
 * compile:
 *  gcc -lssl -loauth -lcurl -o twitter twitter.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oauth.h"


/* constants */
const char *request_token_uri = "https://api.twitter.com/oauth/request_token";
const char *access_token_uri  = "https://api.twitter.com/oauth/access_token";
const char *authorize_uri     = "https://api.twitter.com/oauth/authorize";
const char *req_c_key         = "5JuvuLiIIxC997IRNlADw"; //< consumer key
const char *req_c_secret      = "tmR64sE5qObPe3cNxhzpb3zZMoLKivAqsVlfpJtlM0"; //< consumer secret
const char *friendship_url    = "https://api.twitter.com/1.1/friendships/lookup.json?screen_name=wushaozheng,shaozhengwu";

const char *oauth_token       = "596360126-d0i2HewbLRpl7VRMOjPIJBfLmaAL7eVFGSBaCtY8"; //< consumer key
const char *oauth_token_secret= "fzX8fsyHQ3x02uxj6WwPMN3cu1gA42B5gJbpEmDcAYr6n"; //< consumer secret

/* prototypes */
void access_token_request_data(void);

/* main */
int main (int argc, char **argv)
{

#if 1
    printf(" *** request friendship *** \n\n");

    /* GET a friendship request */
    access_token_request_data();

    /* POST a request-token request */
//    request_token_example_post();
#endif

    return (0);

} /* end of main */



/* sub routines */
/*
 * a example requesting and parsing a request-token from an OAuth service-provider
 * excercising the oauth-HTTP GET function. - it is almost the same as
 * \ref request_token_example_post below.
 */
void access_token_request_data(void) {
    char *req_url = NULL;
    char *reply;

    req_url = oauth_sign_url2(friendship_url, NULL, OA_HMAC,
                              NULL, req_c_key, req_c_secret,oauth_token, oauth_token_secret);

    printf("request URL:%s\n\n", req_url);

    if(req_url) free(req_url);
}
