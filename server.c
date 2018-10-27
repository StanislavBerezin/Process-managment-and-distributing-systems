/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #include "minesweeper.h"
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <arpa/inet.h>

// Server Related Functions
int serverInit(int portNumber);
void serverFunction(int newsockfd);
int authenticate(char *password, char *username, int iSelf);
void authenticateRequest(int iSelf, char query, char buffer, char response);
void assignThread();

//Error handling and Interrupts
void error(const char *msg, int newsockfd);
void fatalerror(const char *msg);
void interruptCallled();

/**Game Related Functions**/
char *gameStart();

char *gameOptions(char rop, char tileA, char tile1, int iSelf);

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

int sockfd, newsockfd;
socklen_t clilen;
struct sockaddr_in cli_addr;

//to exit
struct sigaction instance;

UserRecord userRecords[10];
GameEntry gameEntries[11];

int anyRecords = 0;
int anyGamesPlayed = 0;
int iUserRecords = 0;
int compare_ints(const void *p, const void *q);
char *leaderBoard();

/**Thread Related Functions**/
#define NUMOFTHREADS 2
int buffer[NUMOFTHREADS];
int pointer = -1;

pthread_cond_t cv;
pthread_mutex_t lock;

void threadsInit();
void *threadWorker(void *args);

/**SIGINT Exit Related Functions*/

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
    interruptCallled();
    //Reading the command line args
    sockfd = serverInit(argc < 2 ? 12345 : atoi(argv[1]));

    //Start all the threads
    threadsInit();
    assignThread();

    system("clear");
    printf("Sutting Down Server");
    puts("\n");

    close(sockfd);
    return 0;
}

void interruptCallled()
{
    instance.sa_handler = sigintHandler;
    instance.sa_flags = 0;
    sigemptyset(&instance.sa_mask);
    sigaction(SIGINT, &instance, NULL);
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
    int communicator;

    char buffer[1024];
    char query[1];
    char response[1024];

    //Variable to track the index of player in UserRecords array for LeaderBoard
    int iSelf;

    if (newsockfd < 0)
        error("ERROR on accept", -1);

    while (keepRunning)
    {
        bzero(buffer, 1024);
        //Read the query
        communicator = read(newsockfd, buffer, 255);
        if (communicator < 0)
            error("ERROR reading from socket", newsockfd);
        //printf("Query: %s\n", buffer);
        bzero(query, 1);
        //If query is empty, probably client just shutdown.
        if (strlen(buffer) == 0)
            break;
        bzero(response, 1024);
        switch (buffer[0])
        {
        //'A' for authentication
        case 'A':
            strcat(query, strtok(buffer, ","));

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
            strcat(response, gameOptions(buffer[2], buffer[3], buffer[4], iSelf));
            break;
        }
        // TO DELETE
        printf("Response: %s\n", response);

        //Send the response to client
        communicator = write(newsockfd, response, 1024);

        if (communicator < 0)
            error("ERROR writing to socket", newsockfd);
    }

    close(newsockfd);
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
            strcat(out, print_game());
            strcat(out, "\n\nCongratulations, you have won the match. Time Taken");
            sprintf(out, "%s: %i seconds\n", out, getTime());
            //Add a new entry at the end of Records array.
            gameEntries[10].indexUserRecord = iSelf;
            gameEntries[10].time = getTime();
            //Sort the array as specified.
            qsort(gameEntries, 11, sizeof(GameEntry), &compare_ints);
        }
        else
        {
            strcat(out, reveal_mines());
            strcat(out, "You have lost the game.");
            sprintf(out, "%s. TimeTaken: %i seconds\n", out, getTime());
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
    int i = 0;
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
        for (i = anyRecords - 1; i >= 0; i--)
        {
            UserRecord userRecord = userRecords[gameEntries[i].indexUserRecord];
            sprintf(out, "%s\n%s , %i seconds, %i games won, %i games played", out, userRecord.username, gameEntries[i].time, userRecord.gamesWon, userRecord.gamesPlayed);
        }
    }
    return strdup(out);
}

/**Thread Related Functions*/
void threadsInit()
{
    int i = NUMOFTHREADS;
    pthread_t t[NUMOFTHREADS];
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
