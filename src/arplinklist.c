#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<ctype.h>
#include<arpa/inet.h>

#include "twittrouter.h"
#include "xmalloc.h"

linklist CreatEmptyLink ( )      // creat an empty link;return the head dress of the link
{
	linklist h = (linklist)malloc(sizeof(linknode));
	h->next = NULL;
	return h;
}

static void InsertEmptyLink (linklist p,char* ipaddr,int  ipType) // insert a node at the head of the link
{
	linklist h = (linklist)malloc(sizeof(linknode));
	memset((void *)h, 0, sizeof(linknode));
	strcpy(h -> ipaddr, ipaddr);
    h -> ipType = ipType;
	h -> next = p-> next;
	p -> next = h;
}

linklist Query (linklist p,char* ipaddr ) {
    while (p->next) {
        if (strcmp(p->next->ipaddr,ipaddr) == 0) {
            return p->next;
        }
        p = p->next;
    }
    return NULL;
}

static void CheckList (linklist p) {
    while (p->next) {
        printf("the current list ipaddr == %s,ipType = %d \n",p->next->ipaddr,p->next->ipType);
        p = p->next;
    }
}

void Update (linklist p,int  ipType) {
    p -> ipType = ipType;
}

static bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

static bool isValidMAC(char *s) {
    int i;
    for(i = 0; i < 17; i++) {
        if(i % 3 != 2 && !isxdigit(s[i]))
            return false;
        if(i % 3 == 2 && s[i] != ':' && s[i] != '-')
            return false;
    }
    if(s[17] != '\0')
        return false;
    return true;
}

char* exec_cmd_shell(char *cmd) {
    FILE *in = popen (cmd, "r");
    size_t len = 0;
    size_t alloc = 0;
    char *data = NULL;
    int rcv = 1;
    while (in && rcv > 0 && !feof(in)) {
        alloc +=1024;
        data = (char*)xrealloc(data, alloc * sizeof(char));
        rcv = fread(data + (alloc-1024), sizeof(char), 1024, in);
        len += rcv;
    }
    pclose(in);
    data[len]=0;
    if (data) printf("%s\n",data);
    return data;

}

void scan_arp_and_block(char *arpOutput) {
    char *line = strtok(arpOutput,"\n");
    char *ip;
    char iptable_block_cmd[MAXSTRINGLENGTH] = {'\0',};
    //char iptable_unblock_cmd[MAXSTRINGLENGTH] = {'\0'};

    while (line) {
        line =  strtok(NULL," \t\n");
        if (!line) break;
        //printf("split the line and get %s\n",line);
        if (isValidIpAddress(line)) {
            ip = line;
            continue;
        }
        if (isValidMAC(line)) {
            if(strstr(conf->whitelist,line) || Query(arpList,ip)) {
                continue;
            } else {
                InsertEmptyLink(arpList,ip,BLOCKED_FLAG);
                sprintf(iptable_block_cmd,"iptables -t nat -I PREROUTING -s %s -p tcp --dport 80 -j REDIRECT --to-ports %s", ip, servPort);
                //stmc
                //char *block_cmd_output = exec_cmd_shell(iptable_block_cmd);
                //if(!block_cmd_output) free(block_cmd_output);
                //stmc
                printf("%s\n", iptable_block_cmd);
                printf("blocked ip addr %s \n",ip);
            }
        }
    }

    CheckList(arpList);
}
