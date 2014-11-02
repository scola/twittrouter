#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include "twittrouter.h"
#include "utils.h"
#include "jconf.h"

jconf_t *conf = NULL;
char *root = NULL;
char *servPort = NULL;
linklist arpList = NULL;

int main(int argc, char *argv[]) {
    int auth_flags = 0;
    char c;
    char *conf_path = DEFAULT_CONFIG_PATH;
    char *TwitterID = NULL;
    char *userfortest = NULL;

    root = DEFAULT_SERVER_DIR;
    servPort = DEFAULT_SERVER_PORT;

    opterr = 0;

    while ((c = getopt (argc, argv, "p:r:c:t:u:f:ha")) != -1) {
        switch (c)
        {
            case 'r':
                root = optarg;
                break;
            case 'p':
                servPort = optarg;
                break;
            case 'c':
                conf_path = optarg;
                break;
            case 'u':
                userfortest = optarg;   //it's just used for test oauth
                break;
            case 't':
                TwitterID = optarg;     //it's just used to test the twitter username display on the webpage
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
                break;
            case 'a':
                auth_flags = 1;
                break;    
            default:
                break;
        }
    }

    if (opterr) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (conf_path != NULL)
    {
        conf = read_jconf(conf_path);
        if (TwitterID) conf->TwitterID = TwitterID;
        if (conf->TwitterID == NULL ||
            conf->CONSUMER_KEY == NULL || conf->CONSUMER_SECRET == NULL ||
            conf->OAUTH_TOKEN == NULL || conf->OAUTH_TOKEN_SECRET == NULL) {
            usage();
            exit(EXIT_FAILURE);
        }
    }
    
    if(userfortest != NULL) {
        if(get_friendship(userfortest)) {
            printf("Congratulations! Verify success,%s is your twitter friend\n", userfortest);
        }
        else {
            printf("Sorry! Maybe you have not config your network to go through to evil GFW,or try another friend again\n");
        }
        exit(EXIT_SUCCESS);
    }
    
    if (auth_flags)
    {
        char pin[10] = {0};
        request_token_example_get();
        printf("PIN: ");
        scanf("%s", pin);
        printf("pin=%s\n", pin);
        if(access_token_example_get(pin)) {
            printf("Congratulations! auth success\n");
            dump_jconf(conf_path);
        }
        else {
            printf("Sorry! auth failed\n");
        }
        exit(EXIT_SUCCESS);
    }  
    
    //we need to create a thread to scan arp list and block some ip.
    arpList = CreatEmptyLink();
    LOGD("server listening at port %s...\n",servPort);

    int servSock = SetupTCPServerSocket(servPort);
    if (servSock < 0)
        FATAL("unable to establish");
        
    fd_set readset, errorset;
    int max_fd = servSock + 1;
    int loop_count = 0;
    for (;;) { // Run forever
        FD_ZERO(&readset);
        FD_ZERO(&errorset);
        FD_SET(servSock, &readset);
        FD_SET(servSock, &errorset);
        struct timeval timeout = {
          .tv_sec = 1,
          .tv_usec = 0,
        };
        
        if (-1 == select(max_fd, &readset, NULL, &errorset, &timeout)) {
            ERROR("select");
        }
        
        if (FD_ISSET(servSock, &errorset)) {
            // TODO getsockopt(..., SO_ERROR, ...);
            ERROR("servSock error\n");
        }
        
        if(FD_ISSET(servSock, &readset)) {
            int clntSock = AcceptTCPConnection(servSock);
            HandleTCPClient(clntSock);
        }
        if(loop_count % 10 == 0){
            int fd;
            char buf[BUFSIZE] = {0};
            if ((fd=open("/proc/net/arp", O_RDONLY)) != -1)    //FILE FOUND
            {        
                read(fd, buf, BUFSIZE);
                scan_arp_and_block(buf); 
                close(fd);
            }
            loop_count = 0;
        }
        loop_count++;
    }
    // NOT REACHED
}
