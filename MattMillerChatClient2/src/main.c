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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h> //sockets
#include <unistd.h> //write
#include <errno.h> //errors
#include <string.h> //strings and info
#include <sys/select.h> //found via in class notes

// lets start with something we know... main
int main(){
    
    //Intial info for connecting with the server
    struct sockaddr_in server; //server is variable name of Rodkey's server
    int status, descriptor;
    char name[20], message[80], serverReply[500]; //setting size of name, message, and reply
    
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
    server.sin_port = htons( 49153 );
    inet_pton(AF_INET, "10.115.20.250", &server.sin_addr);
    status=connect(sockfd, (const struct sockaddr *) &server, sizeof(server));
    if(status < 0){
        perror("Connection to server failed...\n");
        return 1;
    }
    puts("Connected to server\n");
    sleep(1); //TODO: Is this needed?
    
    //Ask user for username and then pass it to the server
    puts("Enter Username: ");//TODO reword this?
    fgets(name, sizeof(name), stdin); //fgets is more secure than gets
    send(sockfd, name, strlen(name), 0);
    sleep(3);
    
    //TODO: New code for project II
}
