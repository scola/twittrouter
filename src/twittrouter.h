#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

#include "jconf.h"

#define VERSION "0.1.3"

#define TWITTER_USERNAME_MAX_LEN 20

#define DEFAULT_SERVER_DIR  "/www/twittrouter"
#define DEFAULT_SERVER_PORT "9999"
#define DEFAULT_CONFIG_PATH "/etc/config/twittrouter.json"

typedef struct _node_
{
    char ipaddr[16];
    int ipType;
    struct _node_  *next;

} linknode,*linklist;

void  Update (linklist p,int  ipType);
linklist Query (linklist p,char* ipaddr );
linklist CreatEmptyLink ( );

// Print socket address
char* PrintSocketAddress(const struct sockaddr *address, FILE *stream, int flag);
// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);
// Handle new TCP client
void HandleTCPClient(int clntSocket);
// Get twitter user friendship
bool get_friendship(char *username);
// request oauth url
void request_token_example_get(void);
// get access token
bool access_token_example_get(char *pin);
//execute the shell command
char* exec_cmd_shell(char *cmd);
//scan arp to get connected client and block the one not in the whitelist
void scan_arp_and_block(char *arpOutput);

extern jconf_t *conf;
extern char *root;
extern char *servPort;
extern linklist arpList;

enum sizeConstants {
  MAXSTRINGLENGTH = 128,
  BUFSIZE = 4096,
};

enum ipFlag {
  OAUTHED_FLAG = 0,
  BLOCKED_FLAG = 1,
};

#endif // PRACTICAL_H_
