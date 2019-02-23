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

/*
 *To Run the chat client:
 *
 *Open terminal, navigate to run project folder and
 *type ./main <username>
 *
 *The program will grab replies from the server automatically
 *to send text simply type into the lower box and hit enter.
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

int ret, sockd, sret, len, fd;
char *name, *buffer, *origbuffer;
struct timeval timev;

int fd = 0;

//Select functions
fd_set readfds;

//Replaces my old socket connect as well as server connect
int connect2v4stream(char * srv, int port){
    int ret, sockd;
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
    
    int ret;
    //for some reason changing the ret variable here to any other name and redeclaring it is what breaks
    //the entire thing. It works with or without select() actually be implemented.
    ret = send(fd, msg, strlen(msg), 0);
    if(ret == -1){
        printf("ERROR: trouble sending. errno = %d\n", errno);
        exit(errno);
    }
    return strlen(msg);
}

//replaces old read functionality
void recvandprint(int fd, char *buff){
    //local declaration of ret and sret, can this be moved to the top?
    
    for(;;){
        
        //credit to in-class video: https://www.youtube.com/watch?v=qyFwGyTYe-M
        //sets readfds back to zero upon each loop, clearning it
        FD_ZERO(&readfds);
        //sets readfds to fd, this is done bit by bit
        FD_SET(fd, &readfds);
        
        //memory allocation for buffer
        buff = malloc(BUFSIZE+1);
        ret = recv(fd,buff,BUFSIZE,0);
        if(ret==-1){
            if(errno == EAGAIN){ //calls the same routine at a later time
                break;
            } else {
                //error catching for recieving
                printf("ERROR: error receiving. errno = %d\n", errno);
                exit(errno);
            }
        } else if (ret == 0){
            exit(0);
        } else {
            buff[ret] = 0;
            printf("%s",buff);
        }
        //freeing the buffer
        free(buff);
        
    }
}

int main(int argc, char * argv[]){
    
    //call new connect function
    fd = connect2v4stream(S_IP, S_PORT);
    
    //sets time for 3 seconds, this should maybe be longer.
    //after testing, somewhere between 3-10 seconds is best.
    timev.tv_sec = 5;
    timev.tv_usec = 0;
    //from <sys/socket.h> setsockopt sets the socket options
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    
    //argc or argcount
    if(argc < 2){
        //to run the program use ./main <username>
        printf("ERROR: Correct Usage: ./main <username>\n");
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


