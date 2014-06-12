#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "twittrouter.h"
#include "utils.h"
#include "jconf.h"

jconf_t *conf = NULL;
char *root = NULL;
char *servPort = NULL;
linklist arpList = NULL;

void *ThreadMain(void *arg); // Main program of a thread
void *ScanArpList(void); // a thread to get arp list

// Structure of arguments to pass to client thread
struct ThreadArgs {
    int clntSock; // Socket descriptor for client
};

static void create_thread(void *thread_func,void *threadArgs) {
    pthread_t threadID;
    int returnValue = pthread_create(&threadID, NULL, thread_func, threadArgs);
    if (returnValue != 0)
        FATAL("pthread_create() failed");
    printf("with thread %ld\n", (long int) threadID);
}

int main(int argc, char *argv[]) {
    int pid_flags = 0;
    int auth_flags = 0;
    char c;
    char *conf_path = DEFAULT_CONFIG_PATH;
    char *TwitterID = NULL;
    char *pid_path = NULL;
    char *userfortest = NULL;

    root = DEFAULT_SERVER_DIR;
    servPort = DEFAULT_SERVER_PORT;

    opterr = 0;

    while ((c = getopt (argc, argv, "p:r:c:t:u:hfa")) != -1) {
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
            case 'f':
                pid_flags = 1;
                pid_path = optarg;
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
    
    if (pid_flags)
    {
        demonize(pid_path);
    }
    //we need to create a thread to scan arp list and block some ip.
    arpList = CreatEmptyLink();
    create_thread(ScanArpList,NULL);
    printf("server listening at port %s...\n",servPort);

    int servSock = SetupTCPServerSocket(servPort);
    if (servSock < 0)
        FATAL("unable to establish");
    for (;;) { // Run forever
        int clntSock = AcceptTCPConnection(servSock);

        // Create separate memory for client argument
        struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(
                sizeof(struct ThreadArgs));
        if (threadArgs == NULL)
            FATAL("malloc() failed");
        threadArgs->clntSock = clntSock;

        // Create client thread
        create_thread(ThreadMain,(void *)threadArgs);
    }
    // NOT REACHED
}

void *ThreadMain(void *threadArgs) {
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket file descriptor from argument
    int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs); // Deallocate memory for argument

    HandleTCPClient(clntSock);

    return (NULL);
}

void *ScanArpList(void) {
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());
    sleep(5);
    char *cmd_output = NULL;
    for(;;) {
        char *buf;     
        
        FILE *f = fopen("/proc/net/arp", "r");
        if (f == NULL) FATAL("Invalid arp path.");
        
        fseek(f, 0, SEEK_END);
        long pos = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        buf = malloc(pos + 1);
        if (buf == NULL) FATAL("No enough memory.");
        
        fread(buf, pos, 1, f);
        fclose(f);

        buf[pos] = '\0'; // end of string
        
        //cmd_output = exec_cmd_shell("arp -n");  //
        scan_arp_and_block(buf);
        if(cmd_output) free(buf);
        sleep(10);
    }
}
