/* CAB403 System Programming Assignment 1
    Implemented By:
        KA LONG Lee (Eric) (N9845097)
        Stanislav Berezin (N9694315)
*/

//Sokcet Header file
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
// Standard C Header File
#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>


#define h_addr h_addr_list[0] /* for backward compatibility */

void error(const char *msg)
{
    perror(msg);
    printf("Something is wrong with the Server.\n");
    exit(0);
}

// Function for developing the connection to the server 
void connectToServer(int argc, char * argv[]);
// Client Side Authentication Function 
void displayLoginUI();
void loginFailed();
// Print and Action Function for user selection option
void MenuSwitch();
void gameOptions();
void displayMainMenu();
// Function for quit the game
void exitGame();

// Function for transfer/receive data to/from the server
void sendQueryToServer();
void ReadDataFromServer();

// Variable for socket programing
char buffer[1024];  // A buffer for sending or receiving message to or from the server
int sockfd, portNumber, n;
struct sockaddr_in serv_addr;
struct hostent *server;

int main(int argc, char *argv[])
{
    // Listening the ctrl c commend for quiting the program
    signal(SIGINT, exitGame);
    // Make the server connection
    connectToServer(argc, argv);

    //Once connected to Server, the first thing is to authenticate
    displayLoginUI();

    do{
        // Send the Query to server
        sendQueryToServer();
        // Receive the msg back from the server
        ReadDataFromServer();

        //If there's no response, the server has shut down
        if (buffer[0]=='\0')
            break;
        MenuSwitch();

    } while(true);
    // Kill the socket
    close(sockfd);
    return 0;
}

void MenuSwitch(){
    /*The response is in the format that first two characters "(X)," represent what type of response is that. */
    switch(buffer[0]){
        case 'A':                       // 'A' is for Authentication
            if(buffer[2] == '0') 
                loginFailed();          // Login not successful. Time to shutdown.
            else if (buffer[2] == '1') 
                displayMainMenu();             // Login successful. Move to Main Menu.
            break;

        case 'L':                       // 'L' is for LeaderBoard
            printf("%s", buffer + 2);   // Print the LeaderBoard 
            displayMainMenu();                 // move to main menu
            break;
        
        case 'G':                       // 'G' is for Game Response, the game is not over yet.
            printf("%s", buffer + 2);
            gameOptions();
            break;

        case 'R':                       // 'R' is for result. The game is over
            printf("%s", buffer + 2);
            displayMainMenu();
            break;

        default:
            break;
    }
}

void connectToServer(int argc, char * argv[]){
    // If the command line argument is less than 3
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    // Read the port number from the third posiition of command line argument 
    portNumber = atoi(argv[2]);
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

    serv_addr.sin_port = htons(portNumber);

    // Connecting to the server
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting"); // If it is error for connection
}

void displayLoginUI()
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

    // Read username from client
    printf("Username: ");
    scanf("%s", username);

    // Read Password from client
    printf("Password: ");
    scanf("%s", password);

    bzero(buffer, 1024);

    //Making the query. 
    /*Query is in the same format. First two characters represent what kind of query*/
    //'A' for authentication
    strcat(buffer, "A,");           // buffer = "A,"
    strcat(buffer, username);       // buffer = "A,username"
    strcat(buffer, ",");            // buffer = "A,username,"
    strcat(buffer, password);       // buffer = "A,username,password"

}

// Function for User entering the wrong username or password
// It will kill the app and send the msg to the user
void loginFailed()
{
    // Display the error msg to the client
    puts("You entered an incorrect username or password. Disconnecting..");
    // destroy the socket
    close(sockfd);
    //Quit Program
    exit(0);
}

// Function for Sending the query to server
void sendQueryToServer(){
    //printf("Query: %s\n", buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
}

void ReadDataFromServer(){
    //Clear the buffer and Read the response 
    bzero(buffer,1024);
    n = read(sockfd,buffer,1024);
    if (n < 0)
        error("ERROR reading from socket");
}

void displayMainMenu()
{
    char option; 

    puts("Welcome to the Minesweeper Gaming System.\n");
    puts("Please enter a selection");
    puts("<1> Play Minsweeper");
    puts("<2> Show LeaderBoard");
    puts("<3> Quit\n");

    printf("Selection Option (1-3): ");
    scanf("%c", &option);

    //Validating the input.
    while(option > '3' || option < '1' )
    {
    	printf("Please enter between 1 and 3: \n ");
        scanf("%c", &option);
    }
    
    bzero(buffer, 1024);

    switch(option)
    {
        case '3': // If quit.
            puts("Shutting down");
            close(sockfd);
            exit(0);
            break;
        case '2': // If show leaderboard. 'L' represents leaderboard query.
            bzero(buffer, 1024);
            strcat(buffer, "L");
            break;
        case '1': //If start new game. 'P' is for play a new game.
            bzero(buffer, 1024);
            strcat(buffer, "P");
            break;
    }
}


void gameOptions()
{
    
    char option;             // Variable for recording the option user has choice
    char tileCoordinates[3]; // Variable for the coordinate for the action

    puts("Choose an option:");
    puts("<R> Reveal tile");
    puts("<P> Place Flag");
    puts("<Q> Quit Game");
    puts("Option (R, P, Q): ");

    scanf("%c", &option);

    // Validating the input
    while(option > 'R' || option < 'P' )
    {
       printf("Please select between (R, P, Q): \n");
       scanf("%c", &option);
    }
    // If quit.
    if (option == 'Q')
    {
        displayMainMenu();
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
    // If socket ID is exist
    if (sockfd)
    {
        // Destroy it
        close(sockfd);
    }

    puts("\n============================================================================");
    puts("                                 Was a good game wasn it?!!");
    puts("                                Hope to see you again, bye!");
    puts("==============================================================================");
    exit(1);
}
