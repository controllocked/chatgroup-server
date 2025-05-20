#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <time.h>
#include "sockutils.h"

#define BACKLOG 10
#define EVER ;;
#define MAX_CLIENTS 100
#define MAX_NICK_WAIT 10
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"


struct Client{
    int clientFD;
    int pfd_index;
    char nickname[33];
    int has_nickname;
    time_t time_connected;
};
void kickClient(struct Client* clients[], struct pollfd** pfds, int* pfds_count, int index);
struct Client* acceptIncomingConnection(int serverSocketFD, struct pollfd** pfds, int* pfds_count, int* pfds_size);
void add_to_pfds(struct pollfd *pfds[], int newfd, int *pfds_count, int *fd_size);
void del_from_pfds(struct pollfd *pfds[], int index, int *pfds_count);
void* get_in_addr(struct sockaddr *sa);
int sendall(int s, char* buf, size_t len);
int isValidatedNickname(char* nickname);



int main(int argc, char* argv[]){
    if(argc != 2){ 
        fprintf(stderr, "Wrong arguments");
        exit(EXIT_FAILURE);
    }
    int pfds_size = 5;
    //0 index - server itself
    int pfds_count = 0;
    struct pollfd *pfds = malloc(sizeof *pfds * pfds_size);
    struct Client* clients[MAX_CLIENTS];
    int serverfd;
    char buffer[1024];
    size_t amountReceived;

    struct addrinfo* servinfo = get_addresses(NULL, argv[1], 1);
    if(servinfo == NULL){
        fprintf(stderr, "server: get_addresses error");    
        exit(EXIT_FAILURE);

    }
    serverfd = try_bind(servinfo);
    if(serverfd == -1) {
        fprintf(stderr, "server: try_bind error");    
        exit(EXIT_FAILURE);
    }
    printf("server: server launched...\n");
    if(listen(serverfd, BACKLOG) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // add listener socket to the trackable array
    pfds[0].fd = serverfd;
    pfds[0].events = POLLIN;
    clients[0] = NULL;
    pfds_count = 1;

    printf("server: waiting for incoming connections...\n");

    // main loop 
    for(EVER){
        int poll_count = poll(pfds, pfds_count, -1);
        if(poll_count == -1){
            perror("poll");
            exit(EXIT_FAILURE);
        }
        time_t now = time(NULL);
        
        for(int i = 1; i < pfds_count; i++){
            if(clients[i] && clients[i]->has_nickname == 0 && now - clients[i]->time_connected > MAX_NICK_WAIT){
                printf("server: clientFD %d kicked: nickname input timeout\n", clients[i]->clientFD);
                kickClient(clients, &pfds, &pfds_count, i);
                i--;
            }
        }

        //if any event on any monitored sockets has occured
        for(int i = 0; i < pfds_count; i++){
            if(pfds[i].revents & (POLLIN | POLLHUP)){
                // if server's listener socket is triggered by incoming connection
                if(pfds[i].fd == serverfd){
                    struct Client* client = acceptIncomingConnection(serverfd, &pfds, &pfds_count, &pfds_size);
                    if(client == NULL){
                        fprintf(stderr, "server: acceptIncomingConnection error");
                        exit(EXIT_FAILURE);
                    }
                    clients[pfds_count-1] = client;

                } else{
                    // if client's socket has something to send
                    memset(buffer, 0, sizeof buffer);

                    if((amountReceived = recv(pfds[i].fd, buffer, sizeof(buffer), 0)) <= 0){
                        // if connection is closed or error occured
                        if(amountReceived == 0){
                            printf("server: socket %d hung up\n", pfds[i].fd);
                        } else{
                            perror("recv");
                        }

                        close(pfds[i].fd);
                        del_from_pfds(&pfds, i, &pfds_count);
                        i--;
                    } else{
                        buffer[amountReceived] = '\0';
                        // if server got new message - artificially broadcast it to all clients, excluding server and sender
                        char *ptr_n = strpbrk(buffer, "\r\n");
                        if(ptr_n) *ptr_n = '\0';
                        if(clients[i]->has_nickname == 0){
                            // no nickname
                            if(isValidatedNickname(buffer)){
                                char msg[64];
                                snprintf(msg, sizeof(msg), "Welcome %s%s%s\n", ANSI_COLOR_YELLOW, buffer, ANSI_COLOR_RESET); 
                                int msg_len = strlen(msg);
                                if(sendall(pfds[i].fd, msg, msg_len) == -1) perror("sendall");
                                clients[i]->has_nickname = 1;
                                strncpy(clients[i]->nickname, buffer, sizeof(clients[i]->nickname) - 1);
                            }
                        }
                        else{
                            // has nickname
                            char formatted_msg[1088]; 
                            snprintf(formatted_msg, sizeof(formatted_msg), "%s%s: %s%s%s\n", ANSI_COLOR_YELLOW,clients[i]->nickname, ANSI_COLOR_BLUE, buffer, ANSI_COLOR_RESET);
                            int msg_len = strlen(formatted_msg); 
                        
                            for(int j = 0; j < pfds_count; j++){
                                int dest_fd = pfds[j].fd;
                                if(dest_fd != serverfd && dest_fd != pfds[i].fd){
                                    if(sendall(dest_fd, formatted_msg, msg_len) == -1){
                                        perror("sendall");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //close(clientfd);
    shutdown(serverfd, SHUT_RDWR);
    return 0;
}


struct Client* acceptIncomingConnection(int serverSocketFD, struct pollfd** pfds, int* pfds_count, int* pfds_size){
    char* request_nick = "Enter your nickname: ";
    int msg_len = strlen(request_nick);
    char s[INET6_ADDRSTRLEN];
    int clientfd;
    struct sockaddr_storage clientAddress;
    struct Client* new_client = malloc(sizeof(struct Client));

    socklen_t ca_size = sizeof(clientAddress);

    if((clientfd = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &ca_size)) == -1){
        perror("accept");
        return NULL;
    } else{
        if(sendall(clientfd, request_nick, msg_len) == -1){
            perror("sendall");
            return NULL;
        }
        add_to_pfds(pfds, clientfd, pfds_count, pfds_size);
        printf("server: new connection from %s on socket %d\n",
            inet_ntop(clientAddress.ss_family, get_in_addr((struct sockaddr*)&clientAddress), s, sizeof s),
            clientfd);
    }

    new_client->clientFD = clientfd;
    new_client->pfd_index = *pfds_count;
    new_client->has_nickname = 0;
    new_client->time_connected = time(NULL);
    new_client->nickname[0] = '\0';
    return new_client;
}

void kickClient(struct Client* clients[], struct pollfd** pfds, int* pfds_count, int index){
    char* msg = "You are kicked\n";
    if(sendall(clients[index]->clientFD, msg, strlen(msg)) == -1){
        perror("sendall");
        exit(EXIT_FAILURE);
    }
    close(clients[index]->clientFD);
    free(clients[index]);
    int last_index = *pfds_count-1;
    if(index != last_index){
        clients[index] = clients[last_index];
        clients[index]->pfd_index = index;
    }
    clients[last_index] = NULL;
    del_from_pfds(pfds, index, pfds_count);
}

int isValidatedNickname(char* nickname){
    int i = 0;
    while(nickname[i] != '\0'){
        // if symbol is in range a-z or A-Z
        if(!((nickname[i] >= 'A' && nickname[i] <= 'Z') || (nickname[i] >= 'a' && nickname[i] <= 'z'))) return 0;
        i++;
        if(i > 32) return 0;
    }
    return i > 0;
}
// adds new connection into trackable array
void add_to_pfds(struct pollfd *pfds[], int newfd, int *pfds_count, int *pfds_size){
    if(*pfds_count == *pfds_size){
        *pfds_size *= 2;
        *pfds = realloc(*pfds, sizeof(**pfds) * (*pfds_size));
    }

    (*pfds)[*pfds_count].fd = newfd;
    (*pfds)[*pfds_count].events = POLLIN;
    (*pfds)[*pfds_count].revents = 0;

    (*pfds_count)++;
}

// removes connection from trackable array
void del_from_pfds(struct pollfd *pfds[], int index, int *pfds_count){
    (*pfds)[index] = (*pfds)[*pfds_count - 1];
    (*pfds_count)--; 
}

// transforms general address form to specific one
void* get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// ensures that the message will be fully sent to the client
int sendall(int s, char* buf, size_t len){
    int total = 0;
    int bytesleft = len;
    int n;

    while(total < len){
        if((n = send(s, buf+total, bytesleft, 0)) == -1) break;
        total += n;
        bytesleft -= n;
    }
    return n==-1?-1:0;
}


