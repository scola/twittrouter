#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

#include "jconf.h"

#define VERSION "0.1.0"

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

// Handle error with user msg
void DieWithUserMessage(const char *msg, const char *detail);
// Handle error with sys msg
void DieWithSystemMessage(const char *msg);
// Print socket address
char* PrintSocketAddress(const struct sockaddr *address, FILE *stream, int flag);
// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);
// Handle new TCP client
void HandleTCPClient(int clntSocket);
// Request two twitter user friendship
char* access_token_request_data(char *username);
char* exec_cmd_shell(char *cmd);
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
