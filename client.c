#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
//Sokcet Header file
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define h_addr h_addr_list[0] /* for backward compatibility */

void error(const char *msg)
{
    perror(msg);
    printf("Server has shut down.");
    exit(0);
}

void connectToServer(int argc, char * argv[]);
void gameOptions();
void loginInterface();
void loginFailed();
void mainMenu();
void exitGame();

char buffer[1024];
int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;

int main(int argc, char *argv[])
{
    // int sockfd, portno, n;
    // struct sockaddr_in serv_addr;
    // struct hostent *server;

    signal(SIGINT, exitGame);

    //Once connected to Server, the first thing is to authenticate
    loginInterface();
    do{
        //Send the query to server
        //printf("Query: %s\n", buffer);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");

        //Clear the buffer and Read the response 
        bzero(buffer,1024);
        n = read(sockfd,buffer,1024);
        if (n < 0)
            error("ERROR reading from socket");
        
        //If there's no response, the server has shut down
        if (buffer[0]=='\0')
            break;

        //Parse the response
        //printf("Response: %s\n",buffer);
        /*The response is in the format that first two characters "(X)," represent what type of response is that. */
        //'A' is for authenticate
        if (buffer[0] == 'A')
        {
            switch(buffer[2])
            {
                case '0':
                    //Login not successful. Time to shutdown.
                    loginFailed();
                    break;
                case '1':
                    //Login successful. Move to Main Menu.
                    mainMenu();
                    break;
            }
        }
        //'L' is for leaderboard
        else if (buffer[0] == 'L' )
        {
            //Print the LeaderBoard 
            printf("%s", buffer + 2);
            //and now move to main menu
            mainMenu();
        }
        //'G' is for Game Response, the game is not over yet.
        else if (buffer[0] == 'G')
        {
            printf("%s", buffer + 2);
            gameOptions();
        }
        //'R' is for result. The game is over
        else if (buffer[0] == 'R')
        {
            printf("%s", buffer + 2);
            mainMenu();
        }

    } while(1);
    close(sockfd);
    return 0;
}

void connectToServer(int argc, char * argv[]){
    // If the command line argument is less than 3
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    // Read the port number from the third posiition of command line argument 
    portno = atoi(argv[2]);
    // Create Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) // If Socket Creation is fail
        error("ERROR opening socket");
    
    // get the server IP address by reading the second position of command line argument
    server = gethostbyname(argv[1]);

    if (server == NULL) { // if can not find the server
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    // Using TCP/IP
    serv_addr.sin_family = AF_INET;

    bcopy( (char *)server->h_addr ,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(portno);

    // Connecting to the server
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting"); // If it is error for connection
}

void loginInterface()
{
    char username[20];
    char password[20];


    puts("==========================================================================\n");
    puts("\n");
    puts("            Welcome to the online minesweeper gaming system.             \n ");
    puts("\n");
    puts("===========================================================================");
    puts("You are required to login with your registered username and password to play");
    puts("============================================================================");
    puts("\n");
    puts("                               Please login .                            ");
    puts("\n");
    puts("============================================================================");
    puts("");

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    bzero(buffer, 1024);

    //Making the query. 
    /*Query is in the same format. First two characters represent what kind of query*/
    //'A' for authentication
    strcat(buffer, "A,");
    strcat(buffer, username); strcat(buffer, ",");
    strcat(buffer, password);

}

void loginFailed()
{
    printf("You entered an incorrect username or password. Disconnecting..");
    close(sockfd);
    exit(0);
}

void mainMenu()
{
    int option;

    printf("Welcome to the Minesweeper Gaming System.\n\n");
    printf("Please enter a selection\n");
    printf("<1> Play Minsweeper\n");
    printf("<2> Show LeaderBoard\n");
    printf("<3> Quit\n\n");

    printf("Selection Option (1-3): ");
    //fgets(buff, 10, stdin);
    //option = atoi(buff);
    scanf("%i", &option);
    
    //Validating the input.
    while(option > 3 || option < 1)
    {
    	printf("You entered %i. Please enter between 1 and 3: ", option);
        scanf("%i", &option);
    }
    
    bzero(buffer, 1024);
    switch(option)
    {
        case 3: // If quit.
            printf("Shutting down");
            close(sockfd);
            exit(0);
            break;
        case 2: // If show leaderboard. 'L' represents leaderboard query.
            bzero(buffer, 1024);
            strcat(buffer, "L");
            break;
        case 1: //If start new game. 'P' is for play a new game.
            bzero(buffer, 1024);
            strcat(buffer, "P");
            break;
    }
}

void gameOptions()
{
    char option;
    char tileCoordinates[3];
    printf("Choose an option:\n");
    printf("<R> Reveal tile\n");
    printf("<P> Place Flag\n");
    printf("<Q> Quit Game\n");
    printf("Option (R, P, Q): ");
    scanf("%c", &option);
    //Validating the input
    while(option > 'R' || option < 'P')
    {
       printf("Please select between (R, P, Q):");
       scanf("%c", &option);
    }
    // If quit.
    if (option == 'Q')
    {
        exitGame();
    }
    else //If 'P' aur 'R'
    {
        printf("Enter tile coordinates: ");
        scanf("%2s", tileCoordinates);
	
        while (tileCoordinates[0]<'A' || tileCoordinates[0]>'I' || tileCoordinates[1]<'1' || tileCoordinates[1]>'9')
        {
            printf("Please enter between [1](A-I) and [2](1-9):");
            scanf("%2s", tileCoordinates);
        }
        bzero(buffer, 1024);
        //Query 'G' is for game option selected
        strcat(buffer, "G,");
        strcat(buffer, &option);
        strcat(buffer, tileCoordinates);
    }
}

void exitGame()
{
    if (sockfd)
    {
        close(sockfd);
    }

    puts("\n============================================================================");
    puts("                                 Was a good game wasn it?!!");
    puts("                                Hope to see you again, bye!");
    puts("==============================================================================");
    exit(1);
}
