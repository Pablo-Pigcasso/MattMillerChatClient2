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
 * - Rodkey for the new base code via in class notes: https://westmont.instructure.com/courses/3049/files/folder/Presentations/2019?preview=136453
 * - In-class video for help on select: https://www.youtube.com/watch?v=qyFwGyTYe-M
 * - Rodkey for new send and recv code: https://westmont.instructure.com/courses/3049/files/folder/Presentations/2019?preview=136453
 * - James Bek helped me a lot with helping me figure out how the heck to use ncurses to make windows
 *      - Specifically, James showed me the wonders of scollok.
 * - Dempsey Salazar showed me how to save my cursor position and recall it.
 * - Jared Wilkens helped me remember that sleep() is the answer to everything. And it honestly is the only reason my program works.
 * - http://www.tutorialspoint.com/unix_system_calls/_newselect.htm talks about checking for stdin to change. This was helpful in
 *   finding the...
 * - Second to last result on google page for select read c: http://csapp.cs.cmu.edu/2e/ics2/code/conc/select.c
 *      - This was the straw that broke the camels back when it came to select. This page showed that it is possible to
 *      put two things into read rather than trying to use read and write. Because of this by checking the value in &readfds
 *      in my code, I am able to see if there is anything to read or not.
 * - Tanner Leslie & Jared Wilkens pushed me in the right direction of using ISSET to check the readfds value.
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
//For windows
#include <ncurses.h>

//defines for Server IP, Server PORT,
#define S_IP "10.115.20.250"
//Port numbers we can use are 49153, 49154, 49155
#define S_PORT 49155
#define BUFSIZE 1024

int ret, sockd, sret, len, fd, cury, curx;
char *name, *buffer, *origbuffer;
struct timeval timev;

WINDOW * win; //first window (not box)
WINDOW * win1; //second window (not box)
WINDOW * win2; //big box
WINDOW * win3; //small lower box

int fd = 0;

//Select functions
fd_set readfds;
fd_set writefds;

//buildwindows creates the windows for entering commands, displaying text from server, and for style
void buildwindows(){
    initscr();
    cbreak();
    int strh, strw, height, width, start_y, start_x, h1, w1, st1_y, st1_x, ho1, wo1, sto1_y, sto1_x, ho2, wo2, sto2_y, sto2_x;
    //getmaxyx gets the max size of the screen and stores it in the height and width values respectively
    getmaxyx(stdscr, strh, strw);
    //I am putting two windows inside of a box. To do this I want the outside box to be one row and column
    //bigger on the top, bottom, and both sides. In order to achieve this I take the total size of the screen
    //divide it by two and subtract one from it for height and two for width. In this way each window will be one
    //row shorter, and in total when combined they are two rows shorter and two columns thinner. This gives me one
    //column of space on each side and one row of space on both the top and bottom.
    height = (strh / 2) - 1;
    width = wo2 = strw - 2;
    //*o2 is the parameters for the lower box
    //*o1 is the parameters for the main outside box
    //*1 is the parameters for the lower window inside the lower box
    ho2 = height - 1;
    ho1 = strh;
    wo1 = strw;
    h1 = ho2 - 2;
    w1 = wo2 - 2;
    sto1_y = sto1_x = 0;
    start_y = start_x = 1;
    sto2_y = (strh / 2) + 1;
    sto2_x = 1;
    st1_y = sto2_y+1;
    st1_x = sto2_x+1;
    
    //Creation of each window
    win = newwin(height, width, start_y, start_x);
    win1 = newwin(h1, w1, st1_y, st1_x);
    win2 = newwin(ho1, wo1, sto1_y, sto1_x);
    win3 = newwin(ho2, wo2, sto2_y, sto2_x);
    
    scrollok(win, TRUE);
    //I need at least one refresh basically everywhere.
    refresh();
    
    //boxes are the frames for windows.
    //the first box is the lower window, the second box is the main outside window
    box(win3, 0, 0);
    box(win2, 0, 0);
    /*
     mvwprintw(win, 0, 0, "this is my box1"); //test print
     */
    //refreshes for each of the windows
    wrefresh(win2);
    wrefresh(win);
    wrefresh(win3);
    wrefresh(win1);
    
}

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
    wrefresh(win1);
    cbreak();
    //Don't ask me how. Don't ask me why. But it only works if I refresh as often as I do and I use these specific sleeps
    sleep(1);
    wclear(win1);
    return strlen(msg);
}

//replaces old read functionality
void recvandprint(int fd, char *buff){
    //local declaration of ret and sret, can this be moved to the top?
    
    for(;;){
        
        //Don't ask me how. Don't ask me why. But it only works if I refresh as often as I do and I use these specific sleeps
        sleep(1);
        cbreak();
        wrefresh(win);
        wrefresh(win1);
        
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
            getyx(win1, cury, curx);
            wprintw(win, buff);
        }
        //freeing the buffer
        wmove(win1, cury, curx);
        free(buff);
        wrefresh(win);
        cbreak();
        
    }
}

int main(int argc, char * argv[]){
    
    
    //call new connect function
    fd = connect2v4stream(S_IP, S_PORT);
    
    //sets time for 3 seconds, this should maybe be longer.
    //after testing, somewhere between 3-10 seconds is best.
    timev.tv_sec = 2;
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
    
    buildwindows();
    
    int is_done = 0;
    while(!is_done){
        
        wrefresh(win);
        wrefresh(win1);
        cbreak();
        
        //credit to in-class video: https://www.youtube.com/watch?v=qyFwGyTYe-M
        //sets readfds back to zero upon each loop, clearning it
        FD_ZERO(&readfds);
        //sets readfds to fd, this is done bit by bit
        //http://csapp.cs.cmu.edu/2e/ics2/code/conc/select.c
        //via the above link I can set both the stdin (or 0) to readfds as
        //well as listening (via fd) on readfds. Instead of trying to have one on
        //the read buff and one on the write as I would expect. They both go on
        //the same one. This was super frustrating. Why can't I just use writefds???
        //http://www.tutorialspoint.com/unix_system_calls/_newselect.htm talks about how stdin is 0 and checking for change. This was also helpful.
        FD_SET(fd, &readfds);
        FD_SET(0, &readfds);
        
        //select only needs to use &readfds, the last value should be NULL so that it continually loops
        select(fd + 1, &readfds, NULL, NULL, NULL);
        
        //FD_ISSET checks the value of readfds
        if(FD_ISSET(fd, &readfds)){
            //
            recvandprint(fd,buffer);
        } else if(FD_ISSET(0, &readfds)){
            len = BUFSIZE;
            buffer = malloc(len+1);
            origbuffer = buffer;
            wgetstr(win1, buffer);
            strcat(buffer, "\n");
            //if(getline(&buffer,(size_t *) &len,stdin) > 1){
            sendout(fd, buffer);
            wrefresh(win1);
            //}
            is_done = (strcmp (buffer, "quit\n") == 0);
            free(origbuffer);
        }
    }
    //How windows are ended. I should get here after typing quit.
    endwin();
}
