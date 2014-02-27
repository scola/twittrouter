#ifndef _JCONF_H
#define _JCONF_H

#define MAX_WHITELIST_NUM 10
#define MAX_CONF_SIZE 16 * 1024

typedef struct
{
    char *host;
    char *port;
} remote_addr_t;

typedef struct
{
    char *whitelist;
    char *TwitterID;
    char *CONSUMER_KEY;
    char *CONSUMER_SECRET;
    char *OAUTH_TOKEN;
    char *OAUTH_TOKEN_SECRET;
} jconf_t;

jconf_t *read_jconf(const char* file);

#endif // _JCONF_H
