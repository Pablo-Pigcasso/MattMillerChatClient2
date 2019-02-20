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
}

