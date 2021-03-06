/* CAB403 System Programming Assignment 1
    Implemented By:
        KA LONG Lee (Eric) (N9845097)
        Stanislav Berezin (N9694315)
*/


/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "minesweeper.h"
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <arpa/inet.h>

#define NUMOFTHREADS 10
#define BUFFERSIZE 1024

#define PORTNUM 12345

// Server Related Functions
int serverInit(int portNumber);
void setPort(int argc, char *argv[]);
void serverFunction(int newsockfd);
int authenticate(char *password, char *username, int iSelf);
void authenticateRequest(int iSelf);
void assignThread();

//Error handling and Interrupts
void error(const char *msg, int newsockfd);
void fatalerror(const char *msg);
void interruptCallled();

/**Game Related Functions**/
char *gameStart();
char *gameOptions(char rop, char tileA, char tile1, int iSelf);
int compare_ints(const void *p, const void *q);
char *leaderBoard();

/**LeaderBoard Releated Functions**/
typedef struct
{
    char *username;
    int gamesPlayed;
    int gamesWon;
} UserRecord;

typedef struct
{
    int indexUserRecord;
    int time;
} GameEntry;

UserRecord userRecords[10];
GameEntry gameEntries[11];

int sockfd, newsockfd;
socklen_t clilen;
pthread_t t[NUMOFTHREADS];
pthread_cond_t cv;
pthread_mutex_t lock;
struct sockaddr_in cli_addr;

//to exit

void threadsInit();
void *threadWorker(void *args);
void cleanThread();

/**SIGINT Exit Related Functions*/

// integer for communication between client and server
int communicator;

char bufferREQUEST[BUFFERSIZE];
char query[1];
char response[BUFFERSIZE];
int PORT_NUM = PORTNUM;
int anyRecords = 0;
int anyGamesPlayed = 0;
int iUserRecords = 0;

// Threading
int buffer[NUMOFTHREADS];
int pointer = -1;

//Variable to indicate CTRL-C pressed.
static volatile int keepRunning = 1;

void sigintHandler()
{
    keepRunning = 0;
}

/**Main Function*/
int main(int argc, char *argv[])
{
    system("clear");
    signal(SIGINT, cleanThread);
    //Reading the command line args
    setPort(argc,argv);
    sockfd = serverInit(PORT_NUM);

    //Start all the threads
    threadsInit();
    assignThread();

    system("clear");
    printf("Sutting Down Server");
    puts("\n");

    close(sockfd);
    return 0;
}

// Thread Cleaning
void cleanThread(){
    //Cleaning the Thread
    for (int i = 0; i < NUMOFTHREADS; i++)
    {
        pthread_cancel(t[i]);
        printf("Thread is cleaned : %d \n",i);
    }
    exit(0);
}

void assignThread()
{
    while (keepRunning)
    {
        clilen = sizeof(cli_addr);
        printf("Server: got connection from %s\n", inet_ntoa(cli_addr.sin_addr));
        newsockfd = accept(sockfd,
                           (struct sockaddr *)&cli_addr,
                           &clilen);

        //Put new connection in the buffe
        buffer[++pointer] = newsockfd;
        //Signal to threads that thers is a new connection available.
        pthread_cond_signal(&cv);

        //While all the threads are consumed, new connection is put on wait.
        pthread_mutex_lock(&lock);
        while (pointer >= NUMOFTHREADS)
            pthread_cond_wait(&cv, &lock);
        pthread_mutex_unlock(&lock);
    }
}

/**Server Related Functions*/
int serverInit(int portNumber)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fatalerror("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        fatalerror("ERROR on binding");
    puts("Waiting for players to join...");
    listen(sockfd, 5);

    return sockfd;
}

void serverFunction(int newsockfd)
{
    //Variable to track the index of player in UserRecords array for LeaderBoard
    int iSelf;

    if (newsockfd < 0)
        error("ERROR on accept", -1);

    while (keepRunning)
    {
        bzero(bufferREQUEST, 1024);
        //Read the query
        communicator = read(newsockfd, bufferREQUEST, 255);
        if (communicator < 0)
            error("ERROR reading from socket", newsockfd);
        //printf("Query: %s\n", buffer);
        bzero(query, 1);
        //If query is empty, probably client just shutdown.
        if (strlen(bufferREQUEST) == 0)
        {
            puts("User has disconnected");
            break;
        }

        bzero(response, 1024);

        switch (bufferREQUEST[0])
        {
        //'A' for authentication
        case 'A':
            authenticateRequest(iSelf);
            break;

        //'L' for leaderboard
        case 'L':
            strcat(response, leaderBoard());
            break;
        //'P' for newGameStart
        case 'P':
            anyGamesPlayed++;
            strcat(response, gameStart());
            break;
        //'G' for game option received
        case 'G':
            strcat(response, gameOptions(bufferREQUEST[2], bufferREQUEST[3], bufferREQUEST[4], iSelf));
            break;
        }

        //Send the response to client
        communicator = write(newsockfd, response, 1024);

        if (communicator < 0)
            error("ERROR writing to socket", newsockfd);
    }

    close(newsockfd);
}

void authenticateRequest(int iSelf)
{
    strcat(query, strtok(bufferREQUEST, ","));

    //If more than ten players are connected to the same instance, clear the records for leaderboard
    if (iSelf > 9)
    {
        bzero(userRecords, sizeof(UserRecord) * 10);
        bzero(gameEntries, sizeof(GameEntry) * 11);
        iSelf = 0;
        anyRecords--;
    }

    //Check credentials
    if (authenticate(strtok(NULL, ","), strtok(NULL, ","), iSelf))
    {
        printf("%s has connected", userRecords->username);
        strcat(response, "A,1\n");
    }
    else
    {
        printf("User attempted to connect but failed, disconnecting client...");
        strcat(response, "A,0\n");
    }
}

int authenticate(char *password, char *username, int iSelf)
{

    FILE *fp;
    char buff[255];

    fp = fopen("Authentication.txt", "r");
    fgets(buff, 255, (FILE *)fp);
    //New entry is going to be entered in user records, so track it with iSelf
    iSelf = iUserRecords++;
    userRecords[iSelf].username = strdup(username);

    do
    {
        fscanf(fp, "%s", buff);
        if (strcmp(username, buff) == 0)
        {
            do
            {
                fscanf(fp, "%s", buff);
            } while (buff[0] == ' ');

            if (strcmp(password, buff) == 0)
            {
                fclose(fp);
                return 1;
            }
            else
            {
                break;
            }
        }
    } while (fgets(buff, 255, (FILE *)fp) != NULL && keepRunning);

    fclose(fp);
    return 0;
}

/**Game Related Functions*/
char *gameOptions(char rop, char tileA, char tile1, int iSelf)
{
    char out[1024] = "";
    if (rop == 'P')
    {
        placeFlag(tileA, tile1);
    }
    else if (rop == 'R')
    {
        revealTile(tileA, tile1);
    }

    if (getStatus()) //Check status, if the games is over yet or not.
    {
        strcat(out, "R,");
        anyGamesPlayed--;
        userRecords[iSelf].gamesPlayed++;
        if (getResult()) //If the user has won
        {
            userRecords[iSelf].gamesWon++;
            anyRecords++;
            strcat(out, print_fullGameBroad());
            strcat(out, "\n\nCongratulations, you have won the match. Time Taken");
            sprintf(out, "%s: %i seconds\n", out, getTime());
            //Add a new entry at the end of Records array.
            for (int i = anyRecords - 1; i >= 0; i--)
            {
                gameEntries[i].indexUserRecord = iSelf;
                gameEntries[i].time = getTime();
            }
            //Sort the array as specified.
            qsort(gameEntries, 11, sizeof(GameEntry), &compare_ints);
        }
        else
        {
            strcat(out, print_fullGameBroad());
            strcat(out, "\n\nYou have lost the game. :( \n");
            sprintf(out, "%sTimeTaken: %i seconds\n", out, getTime());
        }
    }
    else
    {
        strcat(out, "G,");
        strcat(out, print_game());
    }

    return strdup(out);
}

char *gameStart()
{
    char out[1024] = "G,";

    //Lock the thread, so that calls to rand() are synchronised
    pthread_mutex_lock(&lock);
    mainGame();
    pthread_mutex_unlock(&lock);
    strcat(out, print_game());

    return strdup(out);
}

/**Leader Board Related Function*/

/* Comparison function. Receives two generic (void) pointers to the items under comparison. */
int compare_ints(const void *p, const void *q)
{
    GameEntry x = *(const GameEntry *)p;
    GameEntry y = *(const GameEntry *)q;

    if (x.time < y.time)
        return -1;
    else if (x.time > y.time)
        return 1;
    else if (x.time == y.time)
    {
        if (userRecords[x.indexUserRecord].gamesWon > userRecords[y.indexUserRecord].gamesWon)
            return -1;
        else if (userRecords[x.indexUserRecord].gamesWon < userRecords[y.indexUserRecord].gamesWon)
            return 1;
        else if (userRecords[x.indexUserRecord].gamesWon == userRecords[y.indexUserRecord].gamesWon)
            return strcmp(userRecords[x.indexUserRecord].username, userRecords[x.indexUserRecord].username);
    }
    return 0;
}

char *leaderBoard()
{
    char out[1024];
    bzero(out, 1024);
    if (anyGamesPlayed != 0) //If any game is being played rightnow.
    {
        sprintf(out, "L,Cannot read the leaderboard while other players are playing\n");
        return strdup(out);
    }
    if (anyRecords == 0) //If none of the game is won yet.
    {
        strcat(out, "L,No records available.\n");
    }
    else
    {
        strcat(out, "L,LeaderBoard\n");
        for (int i = anyRecords - 1; i >= 0; i--)
        {
            UserRecord userRecord = userRecords[gameEntries[i].indexUserRecord];
            printf(" i is currently %d \n", i);
            sprintf(out, "%s\n%s , %i seconds, %i games won, %i games played \n", out, userRecord.username, gameEntries[10].time, userRecord.gamesWon, userRecord.gamesPlayed);
        }
    }
    return strdup(out);
}

/**Thread Related Functions*/
void threadsInit()
{
    int i = NUMOFTHREADS;
    for (i = 0; i < NUMOFTHREADS; i++)
    {
        pthread_create(&t[i], NULL, threadWorker, NULL);
    }
}

void *threadWorker(void *args)
{
    while (keepRunning)
    {
        //Waiting till a new connection is there to be served.
        pthread_mutex_lock(&lock);
        while (pointer < 0)
            pthread_cond_wait(&cv, &lock);
        //Read connection from connections buffer
        int newsockfd = buffer[pointer--];
        pthread_mutex_unlock(&lock);

        //Pass the connection to serverFunction
        serverFunction(newsockfd);
    }
}

//Errors related to one client
void error(const char *msg, int newsockfd)
{
    perror(msg);
    if (!(newsockfd < 0))
        close(newsockfd);
}

//Error on server side. Socket creating and bindings ones.
void fatalerror(const char *msg)
{
    perror(msg);
    exit(0);
}

// Setting up the port
void setPort(int argc, char *argv[])
{
    if (argc == 1)
    {
        puts("\n");
        printf("Setting up default port on: %d", PORT_NUM);
        puts("\n");
    }
    else if (argc == 2)
    {
        int length = strlen(argv[1]);

        if (length > 5)
        {
            puts("\n");
            printf("Error: Try port number with 5 charecters");
            puts("\n");
        }
        else
        {
            PORT_NUM = atoi(argv[1]);
            puts("\n");
            printf("Setting port on port: %d", PORT_NUM);
            puts("\n");
        }
    }
    else
    {
        puts("\n");
        printf("Error: Too many arguments, try again");
        puts("\n");
    }
}