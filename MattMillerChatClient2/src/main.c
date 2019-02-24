/*
 * main.c
 * mattmillerchatclient2
 *
 *  Created on: February 19, 2019
 *      Author: Matthew Miller
 *
 * I started on the 19th because my birthday was the 18th and I was home in colorado for it, telnet was not working.
 *
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
 * - James Bek helped me a lot with helping me figure out how the heck to use ncurses to make windows
 *      - Specifically, James showed me the wonders of scollok.
 * - Dempsey Salazar showed me how to save my cursor position and recall it.
 * - Jared Wilkens helped me figure out threads is easier.
 * - http://man7.org/linux/man-pages/man3/pthread_join.3.html the manual page that made this entire project possible
 */

/*
 *To Run the chat client:
 *
 *to compile use gcc -o main main.c -lncurses
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
//For windows
#include <pthread.h> //to compile add -lncurses after the normal gcc stuff
#include <ncurses.h>

//defines for Server IP, Server PORT,
#define S_IP "10.115.20.250"
#define S_PORT 49153
#define BUFSIZE 1024

//Global declarations
int ret, sockd, len, fd, cury, curx;
char *name, *buffer, *origbuffer;
struct timeval timev;
int is_done;
pthread_t thread1, thread2;

char left[] = " > > disconnected from the server\n";

int fd = 0;

//Initial declarations on the global scale
WINDOW * win; //first window (not box)
WINDOW * win1; //second window (not box)
WINDOW * win2; //big box
WINDOW * win3; //small lower box

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
    ret = send(fd, msg, strlen(msg), 0);
    if(ret == -1){
        printf("ERROR: trouble sending. errno = %d\n", errno);
        exit(errno);
    }
    wrefresh(win1);
    cbreak();
    wclear(win1);
    return strlen(msg);
}

//replaces old read functionality
void recvandprint(int fd, char *buff){
    
    for(;;){
        
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

//recving thread, it basically runs it as its own program, breaks when quit is called
void *recvthread (void *recvthing)
{
    while(!is_done)
    {
        char *buffer;
        recvandprint(fd, buffer);
    }
}

//sending thread, runs on its own until quit is called
void *sendthread (void *sendthing)
{
    while(!is_done)
    {
        int len = BUFSIZE;
        char *buffer = malloc(len+1);
        char *origbuffer = buffer;
        wgetstr(win1, buffer);
        strcat(buffer, "\n");
        sendout(fd, buffer);
        wrefresh(win1);
        //if quit is typed, exit the program gracefully
        is_done = (strcmp (buffer, "quit\n") == 0);
        free(origbuffer);
    }
    sendout(fd, left);
    pthread_cancel(thread2);
    pthread_cancel(thread1);
    endwin();
}

//Main call
int main(int argc, char * argv[]){
    
    //call new connect function
    fd = connect2v4stream(S_IP, S_PORT);
    
    //from <sys/socket.h> setsockopt sets the socket options
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));
    
    //argc or argcount
    if(argc < 2){
        //to run the program use ./main <username>
        printf("ERROR: Correct Usage: ./main <username>\n");
        exit(1);
    }
    
    //sends name out to server
    name = argv[1];
    len = strlen(name);
    name[len] = '\n';
    name[len+1] = '\0';
    sendout(fd, name);
    
    //when called it builds the two boxes and the windows inside them.
    buildwindows();
    
    //makes sure is_done is 0 before starting the loop, probably redudant
    int is_done = 0;
    
    //refreshes the windows before calling the threads
    wrefresh(win);
    wrefresh(win1);
    cbreak();
    
    //calling the threads, the threads have loops in them so they continually run
    pthread_create(&thread1, NULL, recvthread, NULL);
    pthread_create(&thread2, NULL, sendthread, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    //How windows are ended. I should get here after typing quit.
    endwin();
    return 0;
}


