#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "sockutils.h"


void* get_in_addr(struct sockaddr *sa);
void log_connected_address(struct addrinfo*);
int get_sockfd(const char*, const char*);


int main(int argc, char* argv[]){
    char buffer[1024];
    if(argc != 2){
        fprintf(stderr, "Wrong arguments");
        exit(EXIT_FAILURE);
    };

    
    int sockfd = get_sockfd(NULL, argv[1]);
    if(sockfd == -1){
        fprintf(stderr, "get_sockfd error");
        exit(EXIT_FAILURE);
    }

    char* line = NULL;
    size_t lineSize = 0;

    printf("Type message...\n");
    while(1){
        ssize_t charCount = getline(&line, &lineSize, stdin);
        if(charCount > 0){
            if(strcmp(line, "exit\n") == 0) break;
            ssize_t amountSent = send(sockfd, line, charCount, 0);
        }
    }
    close(sockfd);
    return 0;
}



int get_sockfd(const char* ip, const char* port){
    struct addrinfo* servinfo = get_addresses(ip, port, 0);
    if(servinfo == NULL) return -1;

    int sockfd;
    struct addrinfo* succ_addr = NULL;
    sockfd = try_connect(servinfo, &succ_addr);
    if(sockfd == -1 || succ_addr == NULL){
        fprintf(stderr, "client: failed to connect\n");
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo);
    return sockfd;

}

    



