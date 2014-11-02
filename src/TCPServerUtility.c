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
static void handle_http_get(int clntSocket, char* file); 

static char* get_twitter_id(int clntSocket, char *poststr, char *user, ssize_t buffer_len) {
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
    LOGD("content_length_value = %s",content_length_value);
    if (content_length_value[0] == '\0') return NULL;
    int length = atoi(content_length_value);
    if(length - 6 > TWITTER_USERNAME_MAX_LEN) {
        return NULL;
    }

    char *username = strstr(poststr,"uname=");
    char buffer[BUFSIZE] = {'\0',};
    char *crlf = strstr(poststr,"\r\n\r\n");
    LOGD("first recv length = %d", buffer_len);
    if(crlf != NULL && crlf - poststr + 4 + length <= buffer_len && username == NULL) {
        LOGD("expect length = %d", crlf - poststr + 4 + length);
        return NULL;
    }

    if(username == NULL) {
        ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
        if (numBytesRcvd <= 0){
            ERROR("recv() failed");
            return NULL;
        }
        LOGD("get username start");
        LOGD("%s",buffer);
        LOGD("get username finished");
        username = strstr(buffer,"uname=");
        if(username == NULL) return NULL;
    }

    if(length <= 6) {
        handle_http_get(clntSocket, "/VERIFY_FAILED.html");
        return NULL;
    }

    int j = 0;
    char *word = username + 6;
    while(j + 6 < length) {
        if(*(word + j) == '_' ||
        (*(word + j) >= '0' && *(word + j) <= '9') ||
        (*(word + j) >= 'A' && *(word + j) <= 'Z') ||
        (*(word + j) >= 'a' && *(word + j) <= 'z')) {
            user[j] = *(word + j);
            j++;
        } else {
            handle_http_get(clntSocket, "/VERIFY_FAILED.html");
            return NULL;
        }
    }

    LOGD("a friend of twitter %s is verifying...\n", user);
    return user;
}

static void send_to_client(int clntSocket, char *buffer, int send_byte) {
    ssize_t numBytesSent = send(clntSocket, buffer, send_byte, 0);
    if (numBytesSent < 0)
        ERROR("send() failed");
}

static void handle_http_get(int clntSocket, char* file) {
    char data_to_send[BUFSIZE];
    char path[128] = {'\0',};
    int bytes_read;
    int fd;

    strcpy(path, root);
    strcpy(&path[strlen(root)], file);
    LOGD("response to client and send file: %s", path);

    if ((fd=open(path, O_RDONLY)) != -1)    //FILE FOUND
    {
        send_to_client(clntSocket, "HTTP/1.0 200 OK\r\n\r\n", 19); //send header
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
        close(fd);
    }
    else {
        send_to_client(clntSocket, "HTTP/1.0 404 Not Found\r\n\r\n", 26); //FILE NOT FOUND
    }
}

static void handle_http_post(int clntSocket, char *username) {
    if(get_friendship(username)) {
        handle_http_get(clntSocket, "/VERIFY_OK.html");
        sleep(1);
        struct sockaddr_storage localAddr;
        socklen_t addrSize = sizeof(localAddr);
        //char addrBuffer[INET6_ADDRSTRLEN];
        if (getpeername(clntSocket, (struct sockaddr *) &localAddr, &addrSize) < 0)
            ERROR("getsockname() failed");
        char *sock_addr = PrintSocketAddress((struct sockaddr *) &localAddr, stdout, 1);
        if(sock_addr) {
            LOGD("client sock_addr = %s",sock_addr);

            linklist sock_addr_node = Query(arpList,sock_addr);
            if(sock_addr_node && sock_addr_node->ipType == BLOCKED_FLAG) {
                char iptable_unblock_cmd[MAXSTRINGLENGTH] = {'\0',};
                sprintf(iptable_unblock_cmd,"iptables -t nat -D PREROUTING -s %s -p tcp --dport 80 -j REDIRECT --to-ports %s", sock_addr, servPort);
                char *block_cmd_output = exec_cmd_shell(iptable_unblock_cmd);
                if(!block_cmd_output) free(block_cmd_output);
                LOGD("authed client ip address %s",sock_addr);
                Update(sock_addr_node,OAUTHED_FLAG);
            }
            free(sock_addr);
        }
    } else {
        handle_http_get(clntSocket, "/VERIFY_FAILED.html");
    }    
}

int SetupTCPServerSocket(const char *service) {
    // Construct the server address structure
    struct addrinfo addrCriteria;                                     // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_INET;                         // Any address family
    addrCriteria.ai_flags = AI_PASSIVE;                         // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_STREAM;                 // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;                 // Only TCP protocol

    struct addrinfo *servAddr; // List of server addresses
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
        ERROR("getaddrinfo() failed");

    int servSock = -1;
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
        // Create a TCP socket
        servSock = socket(addr->ai_family, addr->ai_socktype,
                addr->ai_protocol);
        if (servSock < 0)
            continue;             // Socket creation failed; try next address
            
        int opt = 1;
        setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        // Bind to the local address and set socket to listen
        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
                (listen(servSock, MAXPENDING) == 0)) {
            // Print local address of socket
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            //char addrBuffer[INET6_ADDRSTRLEN];
            if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
                ERROR("getsockname() failed");
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
        ERROR("accept() failed");

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
    if (numBytesRcvd <= 0){
        ERROR("recv() failed");
        close(clntSocket);
        return;
    }
    LOGD("first recv start\n");
    printf("%s",buffer);
    LOGD("recv finished\n");

    if (strstr(buffer, "HTTP/1.") == NULL)
    {
        send(clntSocket, "HTTP/1.0 400 Bad Request\r\n\r\n", 28, 0);
    } else if (strncmp(buffer, "GET", 3) == 0) {
        char *req_file = strtok (buffer, " \t\n");
        req_file = strtok (NULL, " \t");
        if (req_file == NULL || (strcmp(req_file, "/favicon.ico") && strcmp(req_file, "/twitter-logo-1.png"))) {
            req_file = "/BASEHTML.html";
        }
        handle_http_get(clntSocket, req_file);
    } else if (strncmp(buffer, "POST / ", 7) == 0){
            char friend[TWITTER_USERNAME_MAX_LEN] = {'\0',};
            char *friend_id = get_twitter_id(clntSocket, buffer, friend, numBytesRcvd);
            if(friend_id)
                handle_http_post(clntSocket, friend_id);
    }

    shutdown(clntSocket, SHUT_RDWR); //All further send and recieve operations are DISABLED...
    close(clntSocket); // Close client socket
}
