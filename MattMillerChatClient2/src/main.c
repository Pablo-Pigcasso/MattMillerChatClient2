/*
 * main.c
 * mattmillerchatclient2
 *
 *  Created on: February 19, 2019
 *      Author: Matthew Miller
 *
 * This new file was created when I gave up on trying to just update my old main.c file to cover the new project.
 * Using the notes from class as well as what I learned from the previous project, here is my attempt at project
 * number 2.
 *
 * Acknowledgments:
 */

#include <stdio.h>
#include <time.h> //from class notes
#include <sys/select.h> //from class notes
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h> //sockets
#include <unistd.h> //write
#include <errno.h> //errors
#include <string.h> //strings and info

//NEW CODE
#define SERVER "10.115.20.250"
#define PORT 49153
#define BUFSIZE 1024
#define MESSAGE_S 80

//Intial declarations
//struct sockaddr_in server; //server is variable name of Rodkey's server
struct timeval timev; //timeval is going help time for the while loop
int descriptor, fd, len;
int is_done = 0;
char *name, *buffer, *origbuffer; //setting size of name, message, and reply
//fds from github: https://github.com/dtolj/simple-chat-client-server/blob/master/chat.c
fd_set readfds, testfds, clientfds;
int sockfd, result;
char msg[MESSAGE_S + 1];
char kb_msg[MESSAGE_S + 10];


//new function for connecting to the server
int connect2v4stream(char * srv, int port){
    int ret,sockd;
    struct sockaddr_in sin;
    
    if ((sockd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR: Socket failed to be created correctly. errno = %d\n", errno);
        exit(errno);
    }
    if ((ret = inet_pton(AF_INET, SERVER, &sin.sin_addr)) <= 0){
        printf("ERROR: Trouble converting using inet_pton. return value = %d, errno = %d\n", ret, errno);
        exit(errno);
    }
    
    sin.sin_family = AF_INET; //Used for IPV4
    sin.sin_port = htons(PORT); //Convert port to network endian
    
    if((connect(sockd, (struct sockaddr *) &sin, sizeof(sin))) == -1){
        printf("ERROR: Could not connect to the server. errno = %d\n", errno);
        exit(errno);
    }
    return sockd;
}

//new function to replace send
int sendout(int fd, char *msg){
    
    int ret;
    ret = send(fd, msg, strlen(msg), 0);
    if(ret == -1){
        printf("ERROR: Trouble sending. errno = %d\n", errno);
        exit(errno);
    }
    return strlen(msg);
}

// new receive and print, it grabs all strings from server until there are no more.
void recvandprint (int fd, char *buff){
    
    int ret;
    
    for(;;){
        buff = malloc(BUFSIZE+1);
        ret = recv(fd, buff, BUFSIZE, 0);
        if(ret == -1){
            if(errno == EAGAIN){
                break;
            } else {
                printf("ERROR: error receiving from server. errno = %d\n", errno);
                exit(errno);
            }
        } else if (ret == 0) {
            exit(0);
        } else {
            buff[ret] = 0;
            printf("%s", buff);
        }
        free(buff);
    }
}

// lets start with something we know... main
int main(int argc, char * argv[]){
    
    //NEW CONNECT VIA IN CLASS NOTES
    fd = connect2v4stream( SERVER, PORT);
    
    //Timeout timer, set to half a second
    timev.tv_sec = 0;
    timev.tv_usec = 1000 * 500;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    
    
    if(argc < 2){
        printf("Usage: chat-client Username\n");
        exit(1);
    }
    name = argv[1];
    len = strlen(name);
    name[len] = '\n'; //inserts line break
    name[len+1] = '\0'; //terminates the string
    sendout(fd, name);
    
    //TODO: cite github
    FD_ZERO(&clientfds);
    FD_SET(sockfd,&clientfds);
    FD_SET(0,&clientfds);//stdin
    
    //TODO: Dissect this code.
    while(1) {
        testfds = clientfds;
        select(FD_SETSIZE, &testfds, NULL, NULL, NULL);
        
        for(fd=0;fd<FD_SETSIZE;fd++){
            if(FD_ISSET(fd,&testfds)){
                if(fd==sockfd){
                    result = read(sockfd, msg, MESSAGE_S);
                    msg[result] = '\0';
                    printf("%s", msg + 1);
                    
                    if(msg[0] == 'X'){
                        close(sockfd);
                        exit(0);
                    }
                    
                }
                else if(fd == 0){
                    
                    fgets(kb_msg, MESSAGE_S+1, stdin);
                    if (strcmp(kb_msg, "quit\n") == 0){
                        sprintf(msg, "Disconnecting, shutting down.\n");
                        write(sockfd, msg, strlen(msg));
                        close(sockfd);
                        exit(0);
                    }
                    else {
                        sprintf(msg, "M%s", kb_msg);
                        write(sockfd, msg, strlen(msg));
                    }
                }
            }
        }
    }
    /*    while(!is_done){
     recvandprint(fd, buffer);
     len = BUFSIZE;
     buffer = malloc(len+1);
     origbuffer = buffer;
     if(getline(&buffer,(size_t *) &len,stdin) > 1){
     sendout(fd, buffer);
     }
     is_done = (strcmp (buffer, "quit\n") == 0);
     free(origbuffer);
     }
     
     */
}
