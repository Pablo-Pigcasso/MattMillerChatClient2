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
#define IP "10.115.20.250"
#define PORT 49153
#define BUFSIZE 1024

// lets start with something we know... main
int main(int argc, char * argv[]){
    
    //Intial declarations
    struct sockaddr_in server; //server is variable name of Rodkey's server
    struct timeval timev; //timeval is going help time for the while loop
    int descriptor, fd, len;
    int is_done = 0;
    char *name, message[500], *buffer, *origbuffer; //setting size of name, message, and reply
    
    //Opening socket
    descriptor = socket(PF_INET,SOCK_STREAM, 0); //opens the socket
    if(descriptor == -1){
        perror("Socket creation failed\n");
        return 1;
    }
    puts("Socket Created\n");
    
    //Connecting to the server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server.sin_addr);
    fd=connect(sockfd, (const struct sockaddr *) &server, sizeof(server));
    if(fd < 0){
        perror("Connection to server failed...\n");
        return 1;
    }
    puts("Connected to server\n");
    sleep(1); //TODO: Is this needed?
    
    
    //Timeout timer, set to half a second
    timev.tv_sec = 0;
    timev.tv_usec = 1000 * 500;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    
    
    if(argc < 2){
        printf("Username\n");
        exit(1);
    }
    name = argv[1];
    len = strlen(name);
    name[len] = '\n'; //inserts line break
    name[len+1] = '\0'; //terminates the string
    sendout(fd, name);
    
    //TODO: Dissect this code.
    while(!is_done){
        recvandprint(fd, buffer); //this exists????
        len = BUFSIZE;
        buffer = malloc(len+1);
        origbuffer = buffer;
        if(getline(&buffer,(size_t *) &len,stdin) > 1){
            )
        }
    }
    /*//Ask user for username and then pass it to the server
     puts("Enter Username: ");//TODO reword this?
     fgets(name, sizeof(name), stdin); //fgets is more secure than gets
     send(sockfd, name, strlen(name), 0);
     sleep(3);
     */
    
}

