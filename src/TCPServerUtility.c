#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

#include "twittrouter.h"
#include "jconf.h"
#include "utils.h"

static const int MAXPENDING = 5; // Maximum outstanding connection requests

static char* get_twitter_id(char *poststr, char *user) {
    if(!poststr) return NULL;
    char *content_length = strstr(poststr, "Content-Length");
    if(!content_length) return NULL;
    int i = 0;
    char content_length_value[TWITTER_USERNAME_MAX_LEN] = {'\0',};
    while(*(content_length + 16 + i) != '\n') {
        if(*(content_length + 16 + i) >= '0' && *(content_length + 16 + i) <= '9') {
            content_length_value[i] = *(content_length + 16 + i);
        } else {
            if(*(content_length + 16 + i - 1) >= '0' && *(content_length + 16 + i - 1) <= '9') {
                break;
            }
        }
        i++;
    }
    printf("\n*******content_length_value = %s*******\n",content_length_value);
    if (content_length_value[0] == '\0') return NULL;

    char *username = strstr(poststr,"uname=");
    int j = 0;
    int length = atoi(content_length_value);

    if(length <= 6 || length - 6 > TWITTER_USERNAME_MAX_LEN) return NULL;
    char *word = username + 6;
    while(j + 6 < length) {
        if(*(word + j) == '_' ||
        (*(word + j) >= '0' && *(word + j)) <= '9' ||
        (*(word + j) >= 'A' && *(word + j)) <= 'Z' ||
        (*(word + j) >= 'a' && *(word + j)) <= 'z') {
            user[j] = *(word + j);
            j++;
        } else {
            return NULL;
        }
    }

    printf("a friend of twitter %s is verifying...\n", user);
    return user;
}

static void send_to_client(int clntSocket, char *buffer, int send_byte) {
    ssize_t numBytesSent = send(clntSocket, buffer, send_byte, 0);
    if (numBytesSent < 0)
        DieWithSystemMessage("send() failed");
}

static bool parser_friendship_json(char *friendship) {
    if (!friendship) return false;
    char *first_front = strchr(friendship,'{');
    char *second_front = strrchr(friendship,'{');

    if(first_front && first_front != second_front ) {
        char *connections = strstr(second_front,"connections");
        char *colon = strchr(connections,':');
        if(strncmp(strchr(colon,'"'),"\"none",5) == 0)
            return false;
        else
            return true;
    }else {
        return false;
    }
}

static void handle_http_get(int clntSocket, char* file) {
    char data_to_send[BUFSIZE];
    char path[128] = {'\0',};
    int bytes_read;
    int fd;

    if (strncmp(file, "/\0", 2) == 0)
        file = "/BASEHTML.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

    strcpy(path, root);
    strcpy(&path[strlen(root)], file);
    printf("file: %s\n", path);

    if ((fd=open(path, O_RDONLY)) != -1)    //FILE FOUND
    {
        send_to_client(clntSocket, "HTTP/1.0 200 OK\n\n", 17); //send header
        //send_to_client(clntSocket, "Content-type: text/html\n\n", 25);
        while ((bytes_read=read(fd, data_to_send, BUFSIZE)) > 0) {
            if (strcmp(strrchr(path,'.'), ".html") == 0) {
                char *replaced_id = str_replace(data_to_send, "twitterid", conf->TwitterID);
                if (replaced_id) {
                    send_to_client(clntSocket,replaced_id, strstr(replaced_id,"</html>") - replaced_id + 7);
                    free(replaced_id);
                }

            } else {
                send_to_client(clntSocket,data_to_send, bytes_read);
            }
        }
    }
    else {
        send_to_client(clntSocket, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
    }
    close(fd);
}

static void handle_http_post(int clntSocket, char *username) {
    //char *ip;
    if (!conf) return;
    if (username == NULL) {
        handle_http_get(clntSocket, "/VERIFY_FAILED.html");
        return;
    }
    printf("a friend of twitter %s is verifying...\n", username);

    char *friendship = access_token_request_data(username);
    if(friendship) {
        if(parser_friendship_json(friendship)) {

            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            //char addrBuffer[INET6_ADDRSTRLEN];
            if (getpeername(clntSocket, (struct sockaddr *) &localAddr, &addrSize) < 0)
                DieWithSystemMessage("getsockname() failed");
            //char *sock_addr = PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
            char *sock_addr = PrintSocketAddress((struct sockaddr *) &localAddr, stdout, 1);
            if(sock_addr) {
                printf("********client sock_addr = %s***********\n",sock_addr);

                linklist sock_addr_node = Query(arpList,sock_addr);
                if(sock_addr_node && sock_addr_node->ipType == BLOCKED_FLAG) {
                    char iptable_unblock_cmd[MAXSTRINGLENGTH] = {'\0'};
                    sprintf(iptable_unblock_cmd,"iptables -t nat -D PREROUTING -s %s -p tcp --dport 80 -j REDIRECT --to-ports %s", sock_addr, servPort);
                    char *block_cmd_output = exec_cmd_shell(iptable_unblock_cmd);
                    if(!block_cmd_output) free(block_cmd_output);
                    printf("authed client ip address %s \n",sock_addr);
                    Update(sock_addr_node,OAUTHED_FLAG);
                }
                free(sock_addr);
            }
            handle_http_get(clntSocket, "/VERIFY_OK.html");

        } else {
            handle_http_get(clntSocket, "/VERIFY_FAILED.html");
        }
        free(friendship);
    }
}

int SetupTCPServerSocket(const char *service) {
    // Construct the server address structure
    struct addrinfo addrCriteria;                                     // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;                         // Any address family
    addrCriteria.ai_flags = AI_PASSIVE;                         // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_STREAM;                 // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;                 // Only TCP protocol

    struct addrinfo *servAddr; // List of server addresses
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
        DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

    int servSock = -1;
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
        // Create a TCP socket
        servSock = socket(addr->ai_family, addr->ai_socktype,
                addr->ai_protocol);
        if (servSock < 0)
            continue;             // Socket creation failed; try next address

        // Bind to the local address and set socket to listen
        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
                (listen(servSock, MAXPENDING) == 0)) {
            // Print local address of socket
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            //char addrBuffer[INET6_ADDRSTRLEN];
            if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
                DieWithSystemMessage("getsockname() failed");
            fputs("Binding to ", stdout);
            PrintSocketAddress((struct sockaddr *) &localAddr, stdout, 0);
            fputc('\n', stdout);
            break;             // Bind and listen successful
        }

        close(servSock);    // Close and try again
        servSock = -1;
    }

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    return servSock;
}

int AcceptTCPConnection(int servSock) {
    struct sockaddr_storage clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    //char addrBuffer[INET6_ADDRSTRLEN];
    if (clntSock < 0)
        DieWithSystemMessage("accept() failed");

    // clntSock is connected to a client!

    fputs("Handling client ", stdout);
    //PrintSocketAddress((struct sockaddr *) &clntAddr, stdout, addrBuffer);
    PrintSocketAddress((struct sockaddr *) &clntAddr, stdout,0);
    fputc('\n', stdout);

    return clntSock;
}

void HandleTCPClient(int clntSocket) {
    char buffer[BUFSIZE]; // Buffer for echo string

    // Receive message from client
    ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
    if (numBytesRcvd < 0)
        DieWithSystemMessage("recv() failed");
    if (numBytesRcvd == 0)
        DieWithSystemMessage("Client disconnected upexpectedly");
    printf("**********recv start**********\n");
    printf("%s",buffer);
    printf("**********recv finished**********\n");

    //char *username = strstr(buffer,"uname=");
    char friend[TWITTER_USERNAME_MAX_LEN] = {'\0',};
    char *reqline[3];

    char *friend_id = "shaozhengwu";
    reqline[0] = strtok (buffer, " \t\n");
    reqline[1] = strtok (NULL, " \t");
    reqline[2] = strtok (NULL, " \t\n");

    if (reqline[2]== NULL || (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp( reqline[2], "HTTP/1.1", 8) != 0))
    {
        send(clntSocket, "HTTP/1.0 400 Bad Request\n", 25, 0);
    } else if (strncmp(reqline[0], "GET\0", 4)==0) {
        handle_http_get(clntSocket, reqline[1]);
    } else if (strncmp(reqline[0], "POST\0", 5)==0){
        handle_http_post(clntSocket, friend_id);
    }

    close(clntSocket); // Close client socket
}
