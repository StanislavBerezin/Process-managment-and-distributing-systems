#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

//Sokcet Header file
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define NAME_LENGTH 7
#define PASS_LENGTH 6
#define MAXBUFFERSIZE 512
#define h_addr h_addr_list[0]

char username[NAME_LENGTH];
char password[PASS_LENGTH];

int minesRemained;

// functions declarations
void welcomeToGameScreen();

void connectToServer(int *argc, char *argv[]);
void sendMsgToServer();
void ReceiveMsgFromServer();

int authenticate();
void selectMode();
void displayMenu();
void quitGame();
void checkExit(char *word);

//Connection Variable
int network_socket, flag;
int login;
int PORT_NUM;
struct hostent *he;

// Specify an address for the socket
struct sockaddr_in server_address;

// Variable for sending msg to the server

char buffer[MAXBUFFERSIZE];

// Main function
int main(int argc, char *argv[])
{
    connectToServer(&argc, argv);

    welcomeToGameScreen();
    while (login == 0)
    {
        login = authenticate();
    }

    while (1)
    {
        displayMenu();
    }
    return 1;
}

// need to send details to server from char username and password
int authenticate()
{
    printf("Please insert your username: ");
    scanf("%s", username);
    printf("Please insert your password: ");
    scanf("%s", password);
    sprintf(buffer, "%s %s", username, password);
    sendMsgToServer();
    ReceiveMsgFromServer();

    // strcmp compares, if == 0, then equal, if negative S1 < S2, if positive S1>S2
    if (strcmp(buffer, "login success") == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// When user connected to the server, they can insert :exit to kill the connection
void checkExit(char *msg)
{
    // If user enter :exit
    if (strcmp(msg, ":exit") == 0)
    {
        // Close the socket port
        close(network_socket);
        printf("[-] Disconnected from server. \n");
        // End Program
        exit(1);
    }
}

// Print Server response msg on the terminal
void sendMsgToServer()
{
    // Send msg to the network socket
    // You only need to change the value of buffer outside
    if (send(network_socket, buffer, MAXBUFFERSIZE, 0) == -1)
    {
        printf("[-] Error in sending data. \n");
        close(network_socket);
        exit(1);
    }
}

void ReceiveMsgFromServer()
{
    // If we can not receive msg from the server
    if (recv(network_socket, buffer, MAXBUFFERSIZE, 0) < 0)
    {
        printf("[-] Error in receiving data. \n");
        close(network_socket);
    }
    else
    {
        printf("Server : \t%s\n", buffer);
    }
}

// connect to server
void connectToServer(int *argc, char *argv[])
{

    // Need to improve this one
    if ((he = gethostbyname(argv[1])) == NULL)
    { /* get the host info */
        printf("[-]get host by name\n");
        exit(1);
    }

    if (*argc != 3)
    {
        printf("[-]usage: ./client hostname port\n");
        exit(1);
    }

    // Read PORT Number from argv
    PORT_NUM = atoi(argv[2]);

    // First Parameter is the domain of the socket -> Becuase it is internet socket so it is AF_INET
    // Second Parameter SOCK_STEAM = Using TCP
    // thrid Parameter is define the protocol we use 0 for TCP
    if ((network_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("[-] Socket Creation fail... \n");
        exit(1);
    }
    // Becuase it is internet socket so it is AF_INET
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    server_address.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(server_address.sin_zero), 8);

    // First Parameter is the actual socket
    int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    // Check for error with the connection
    if (connection_status == -1)
    {
        printf("[-]There was an error making a connection to the remote socket \n\n\n");
        exit(1);
    }

    printf("[+]Server Connected ! \n");
}

// Function definitions
void displayMenu(void)
{

    char command[32];

    puts("Please enter a selection:\n");
    puts("<1> Play Game");
    puts("<2> Show Leaderboard");
    puts("<3> Quit\n");
    printf("Enter an option (1-3): ");
    scanf("%s", command);
    command[1] = '\0';

    selectMode(atoi(command));
}

void selectMode(int commandId)
{
    switch (commandId)
    {
    case 1:
        // hangman();
        break;
    case 2:
        // leaderboard();
        break;
    case 3:
        quitGame();
        break;
    default:
        displayMenu();
        break;
    }
}

// COMPONENTS FOR DISPAY
void printChoice()
{
    printf("\n");
    printf("Choose an aption: \n");
    printf("\n");
    printf("<R> Reveal tile \n");
    printf("<P> Place flag \n");
    printf("<Q> Quit game \n");
    printf("\n");
}

void welcomeToGameScreen()
{
    puts("=======================================================================\n");
    puts("Welcome to the online minesweeper gaming system.\n ");
    puts("\n");
    puts("=======================================================================\n \n");
    puts("You are required to login with your registered username and password to play");
    puts("========================================================================== ===");
    puts("\n");
    puts("                                  Please login to the server.");
    puts("\n");
    puts("==============================================================================");
    puts("");
}

void remainingMines()
{
    printf("Remaining mines: %d\n", minesRemained);
}

void quitGame()
{
    close(network_socket);
    puts("\n============================================================================");
    puts("                                 Was a good game wasn it?!!");
    puts("                                Hope to see you again, bye!");
    puts("==============================================================================");
    exit(1);
}