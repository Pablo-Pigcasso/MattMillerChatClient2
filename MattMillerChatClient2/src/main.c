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
 *
 * Acknowledgments:
 * - In-class video for help on select: https://www.youtube.com/watch?v=qyFwGyTYe-M
 * - Rodkey for new send and recv code: https://westmont.instructure.com/courses/3049/files/folder/Presentations/2019?preview=136453
 * - I'm sure there is some stack overflow I could cite, but none of them actually helped. Just gave me a better
 * understanding of what I was looking at overall. Not one in particular saved the day though.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
//For select
#include <sys/select.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO

//defines for Server IP, Server PORT,
#define S_IP "10.115.20.250"
#define S_PORT 49153
#define BUFSIZE 1024

int len, ret, sockd, sret;
char *name, *buffer, *origbuffer;
struct timeval timev;

int fd = 0;

//Select functions
fd_set readfds;

//Replaces my old socket connect as well as server connect
int connect2v4stream(char * srv, int port){
    struct sockaddr_in sin;
    
    //creates socket for connection to the server
    if((sockd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR: Could not create socket. errno = %d\n", errno);
        exit(errno); //Error catching if socket returns the failed creation code -1
    }
    
    //inet p to n converter.
    if((ret = inet_pton(AF_INET, S_IP, &sin.sin_addr)) <= 0){
        printf("ERROR: could not correctly convert using inet_pton. \n Return value = %d, errno = %d\n", ret, errno);
        exit(errno);
    }
    
    sin.sin_family = AF_INET;
    sin.sin_port = htons(S_PORT);
    
    //connects to the server
    if((connect(sockd, (struct sockaddr *) &sin, sizeof(sin))) == -1){
        printf("ERROR: trouble connecting to server. errno = %d\n", errno);
        exit(errno);
    }
    return sockd;
}

//Replaces my old fgets, honestly this is a lot cleaner.
int sendout(int fd, char *msg){
    //TODO: check that changing ret to rets did not break anything
    int rets;
    rets = send(fd, msg, strlen(msg), 0);
    if(rets == -1){
        printf("ERROR: trouble sending. errno = %d\n", errno);
        exit(errno);
    }
    //Do I need to return message after sending it? Shouldn't my read do this already?
    //TODO: check to see if I even need this
    return strlen(msg);
}

//replaces old read functionality
void recvandprint(int fd, char *buff){
    //local declaration of ret and sret, can this be moved to the top?
    int ret;
    int sret;
    for(;;){
        
        //sets readfds back to zero upon each loop, clearning it
        FD_ZERO(&readfds);
        //sets readfds to fd, this is done bit by bit
        FD_SET(fd, &readfds);
        
        //select implemented
        //credit to in-class video: https://www.youtube.com/watch?v=qyFwGyTYe-M
        sret = select(8, &readfds, NULL, NULL, &timev);
        
        if(sret == 0){
            printf("sret = %d\n",sret);
            printf("    timeout\n");
        } else {
            printf("sret = %d\n",sret);
            buff = malloc(BUFSIZE+1);
            ret = recv(fd,buff,BUFSIZE,0);
            if(ret==-1){
                if(errno == EAGAIN){
                    break;
                } else {
                    printf("ERROR: error receiving. errno = %d\n", errno);
                    exit(errno);
                }
            } else if (ret == 0){
                exit(0);
            } else {
                buff[ret] = 0;
                printf("%s",buff);
            }
            free(buff);
        }
    }
}

int main(int argc, char * argv[]){
    
    fd = connect2v4stream(S_IP, S_PORT);
    
    timev.tv_sec = 5;
    timev.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    
    if(argc < 2){
        printf("Usage: Networks chat-client <username>\n");
        exit(1);
    }
    name = argv[1];
    len = strlen(name);
    name[len] = '\n';
    name[len+1] = '\0';
    sendout(fd, name);
    
    
    int is_done = 0;
    while(!is_done){
        
        recvandprint(fd,buffer);
        
        len = BUFSIZE;
        buffer = malloc(len+1);
        origbuffer = buffer;
        if(getline(&buffer,(size_t *) &len,stdin) > 1){
            sendout(fd, buffer);
        }
        is_done = (strcmp (buffer, "quit\n") == 0);
        free(origbuffer);
    }
}






















