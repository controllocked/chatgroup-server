#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct addrinfo* get_addresses(const char* ip, const char* port, int s){
    int rv;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if(s == 0) hints.ai_flags = 0;
    else hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(ip, port, &hints, &servinfo)) == -1){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }
    return servinfo;

}


int try_connect(struct addrinfo* servinfo, struct addrinfo** out){
    int sockfd;
    struct addrinfo* p;

    for(p = servinfo; p != NULL; p=p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("cleint: socket error\n");
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            if(close(sockfd) == -1){
                perror("client: failed to close socket");
                return -1;
            }
            perror("client: connect\n");
            continue;
        }
        *out = p;
        return sockfd;
    }
    return -1;
}


int try_bind(struct addrinfo* servinfo){
    struct addrinfo* p;
    int yes = 1;
    int sockfd;
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket error");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            return -1;
        }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("server: binding error");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        return -1;
    }
    return sockfd;
}

