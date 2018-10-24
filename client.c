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

#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NAME_LENGTH 7
#define PASS_LENGTH 6
#define MAXBUFFERSIZE 512
#define h_addr h_addr_list[0]

char username[NAME_LENGTH];
char password[PASS_LENGTH];

int minesRemained = 10;

// functions declarations
void welcomeToGameScreen();
// SERVER
void connectToServer(int *argc, char *argv[]);
void sendMsgToServer(char *msg);
void ReceiveMsgFromServer();

// CLIENT ORINTATED
int authenticate();
void displayMenu();
void selectMode();
void gameInit();

// Game Function
void displayGameBroad();

//Client Game Function
void receiveGameBroad();

// exitFun
void quitGame();
void exitGame();
void checkExit();

// DISPLAY
void printChoice();

//Connection Variable
int network_socket,
    flag;
int login;
int PORT_NUM;
struct hostent *he;


// CLIENT GAME VARIABLES
bool is_selecting_mode = true;
bool is_game_on = true;
// Specify an address for the socket
struct sockaddr_in server_address;

char letters[NUM_TILES_Y] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'};
// Variable for sending msg to the server

char buffer[MAXBUFFERSIZE];

// Main function
int main(int argc, char *argv[])
{
    signal(SIGINT, exitGame);
    connectToServer(&argc, argv);
    welcomeToGameScreen();

    while (login == 0)
    {
        login = authenticate();
    }

    while(is_game_on){
        receiveGameBroad();
        if (strcmp(buffer, "Selection option (1-3) :") == 0){
            scanf("%s",buffer);
            sendMsgToServer(buffer);
        }

        // if (strcmp(buffer, "insertCoordinate" == 0)){

        // }
    }

    return 1;
}

// Print Server response msg on the terminal
void sendMsgToServer(char *msg)
{
    // Copy the msg to the buffer in order to send it to server
    strcpy(buffer, msg);
    checkExit();
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
        exit(1);
    }
    else
    {
        // Return Server Response on Screen
        printf("Server : %s\n", buffer);
    }
}

// THIS is where the game begins, we send to server 1-game-mode, to tell that the game
// has started, there is no functionality developed yet, but it already notifies it

void gameInit()
{
    char *notifyServer = "1-game-mode";
    sprintf(buffer, "%s", notifyServer);
    sendMsgToServer(buffer);
    system("clear");

    while (is_game_on)
    {
        char input[512];
        printf("Remaining mines: %d", minesRemained);
        printf("\n");
        printf("\n");
        printf("    ");
        for (int i = 0; i <= NUM_TILES_X; i++)
        {
            printf("%d ", i);
        }
        printf("\n");
        printf("    ");
        for (int i = 0; i <= NUM_TILES_X; i++)
        {
            printf("- ");
        }
        printf("\n");

        for (int is = 0; is < NUM_TILES_Y; is++)
        {
            //write the X coordinate to the start of the line
            printf("%c |\t", letters[is]);

            printf("\n");
        }
        printf("\n");
        printChoice();
        printf("\n");
        printf("Option (R, P, Q): ");

        scanf("%s", input);

        sprintf(buffer, "%s", input);

        sendMsgToServer(buffer);

        if (strcmp(input, "R") == 1)
        {
            printf("num 2");
        }
    }
}

void selectMode(int commandId)
{
    while (is_selecting_mode)
    {
        switch (commandId)
        {
        case 1:
            gameInit();
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
}
// COMPONENTS FOR DISPAY
void printChoice()
{
    printf("\n");
    printf("Choose an option: \n");
    printf("\n");
    printf("<R> Reveal tile \n");
    printf("<P> Place flag \n");
    printf("<Q> Quit game \n");
    printf("\n");
}

void welcomeToGameScreen()
{
    system("clear");

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
void quitGame()
{
    close(network_socket);
    puts("\n============================================================================");
    puts("                                 Was a good game wasn it?!!");
    puts("                                Hope to see you again, bye!");
    puts("==============================================================================");
    exit(1);
}
void exitGame()
{
    if (network_socket)
    {
        char *bye = "exit";
        sprintf(buffer, "%s", bye);
        sendMsgToServer(buffer);
        close(network_socket);
    }
    exit(1);
}

void checkExit()
{
    // If user enter :exit
    if (strcmp(buffer, ":exit") == 0)
    {
        // Close the socket port
        close(network_socket);
        printf("[-] Disconnected from server. \n");
        // End Program
        exit(1);
    }
}

// connect to server
void connectToServer(int *argc, char *argv[])
{

    // Need to improve this one
    if ((he = gethostbyname(argv[1])) == NULL)
    { /* get the host info */
        printf("Error: get host by name\n");
        exit(1);
    }

    if (*argc != 3)
    {
        printf("Error: usage: ./client hostname port\n");
        exit(1);
    }

    // Read PORT Number from argv
    PORT_NUM = atoi(argv[2]);

    // First Parameter is the domain of the socket -> Becuase it is internet socket so it is AF_INET
    // Second Parameter SOCK_STEAM = Using TCP
    // thrid Parameter is define the protocol we use 0 for TCP
    if ((network_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error: Socket Creation fail... \n");
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
        printf("Error: There was an error making a connection to the remote socket \n\n\n");
        exit(1);
    }

    printf("Server Connected ! \n");
}

// need to send details to server from char username and password
int authenticate()
{
    printf("Please insert your username: ");
    scanf("%s", username);
    printf("Please insert your password: ");
    scanf("%s", password);

    // Copy the message into buffer
    sprintf(buffer, "%s %s", username, password);

    sendMsgToServer(buffer);
    system("clear");
    ReceiveMsgFromServer();
    puts("\n");

    // If the Server return "login success" msg to the buffer
    if (strcmp(buffer, "Welcome to minesweeper") == 0)
    {
        //terminate the authenticate function
        // strcmp compares, if == 0, then equal, if negative S1 < S2, if positive S1>S2

        return 1;
    }
    else
    {

        return 0;
    }
}


// ************************* Game Function ********************************

void receiveGameBroad(){
    // If we can not receive msg from the server
    if (recv(network_socket, buffer, MAXBUFFERSIZE, 0) < 0)
    {
        printf("[-] Error in receiving data. \n");
        close(network_socket);
        exit(1);
    }
    else
    {
        // Return Server Response on Screen
        printf("%s \n", buffer);
    }
}