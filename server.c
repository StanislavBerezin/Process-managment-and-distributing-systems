
// imports
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

//Sokcet Header file
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

#define AUTH_TXT "Authentication.txt"
#define MAXBUFFERSIZE 512

#define RANDOM_NUMBER_SEED 42
#define MINES_NUMBER 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

//Connection
int PORT_NUM;
int client_socket, server_socket;
int user_list_count; //Counter for the AuthTxt file
struct sockaddr_in client_address, server_address;
int loggedIn;

//Process
pid_t childpid;

// Variable
char buffer[MAXBUFFERSIZE];

// functions declarations
void readAuthTxt();
void init();
void sendMsgToClient();
void ReceiveMsgFromClient();

void exitGame();
void ServerSetUP();
int CheckLoginDetails();
void SendWelcomeMsg();

struct User
{
    char *username;
    char *password;
} * users;

// Main function

int main(int argc, char *argv[])
{
    // Save PORT NUM from terminal argc
    if (argc != 2)
    {
        printf("[-] Usage: server port_number \n");
        exit(1);
    }
    // ASCII to Int for PORT Number
    PORT_NUM = atoi(argv[1]);

    signal(SIGINT, exitGame);
    readAuthTxt();
    ServerSetUP();

    while (loggedIn == 0)
    {
        loggedIn = CheckLoginDetails();
    }

    // if ( (childpid = fork() ) == 0){
    //     close(server_socket);
    //     CheckLoginDetails();
    // }
    // free(buffer);
    // Close the socket connection
    close(server_socket);
    return 1;
}

int CheckLoginDetails()
{

    ReceiveMsgFromClient();

    char *username = strtok(buffer, " ");
    char *password = strtok(NULL, "");

    printf("Seperated String from buffer: username: %s, password: %s \n", username, password);

    for (int i = 1; i < user_list_count; i++)
    {

        printf("Users[%d] - username = %s , password = %s \n\n", i, users[i].username, users[i].password);

        if (strcmp(users[i].username, username) == 0)
        {

            if (strcmp(users[i].password, password) == 0)
            {
                // S1 copies buffer
                strcpy(buffer, "login success");
                sendMsgToClient();
                return 1;
            }
        }
    }

    strcpy(buffer, "failed");
    sendMsgToClient();

    return 0;
}

void sendMsgToClient()
{
    // Send msg to the network socket
    // You only need to change the value of buffer outside
    if (send(client_socket, buffer, MAXBUFFERSIZE, 0) == -1)
    {
        printf("[-] Error in sending data. \n");
        close(client_socket);
        exit(1);
    }
}

void ReceiveMsgFromClient()
{
    // If we can not receive msg from the server
    if (recv(client_socket, buffer, MAXBUFFERSIZE, 0) < 0)
    {
        printf("[-] Error in receiving data. \n");
        close(client_socket);
        exit(1);
    }
    else
    {
        printf("client : \t%s\n", buffer);
    }
}

// Function definitions
// Send Message back to Client
void ServerSetUP()
{
    // Notes: It is just a testing function, will change later on

    socklen_t addr_size;

    // Server Socket init
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    // Check Socket Connection
    if (server_socket < 0)
    {
        printf("[-]Error in connection \n");
        exit(1);
    }

    printf("[+]Server Socket is created. \n");

    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    // basically will result to any IP address on the local machine
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to our specified IP and port
    int ret = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (ret < 0)
    {
        printf("[-]Fail to bind to specified IP and port \n");
        exit(1);
    }

    printf("[+]Bind successfully to port %d \n ", PORT_NUM);

    printf("[+]Start listening to the client!!! \n");
    // *** Important *** second argument is a backlog which is
    // how many connection can be waiting for this particular socket at one point in time
    if (listen(server_socket, 10) == 0)
    {
        printf("[+]Listening.... \n");
    }
    else
    {
        printf("[-]Error in binding. \n");
        exit(1);
    }

    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &addr_size);

    // Might need to change something, always failed here
    if (client_socket < 0)
    {
        printf("[-] Client Connection Failed ");
        exit(1);
    }

    printf("[+]Connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
}

void readAuthTxt()
{

    FILE *file_pointer;
    // Open a stream to the file
    file_pointer = fopen(AUTH_TXT, "r");

    user_list_count = 1;
    int ch = 0;

    while ((ch = fgetc(file_pointer)) != EOF)
    {
        if (ch == '\n')
        {
            user_list_count++;
        }
    }

    rewind(file_pointer);

    users = malloc(sizeof(struct User) * user_list_count);

    for (int i = 0; i < user_list_count; i++)
    {
        char username[64], password[64];
        fscanf(file_pointer, "%s", username);
        fscanf(file_pointer, "%s", password);
        users[i].username = malloc(strlen(username) + 1);
        users[i].password = malloc(strlen(password) + 1);
        strcpy(users[i].username, username);
        strcpy(users[i].password, password);
    }

    fclose(file_pointer);
}

// initialising game

void initGame()
{
    srand(RANDOM_NUMBER_SEED);
}

// exit the game and free resources
void exitGame()
{
    printf("\n\n Ctrl+c was pressed, caused an interupt, closing connection \n\n");

    // will need to free memory here
    close(server_socket);
    close(client_socket);
    printf("Memory has been freed, sockets and memory are down\n");
    exit(1);
}

// code for allocating mines
// void placeMines()
// {
//     for (int i = 0; i < MINES_NUMBER; i++)
//     {
//         int x, y;

//         do
//         {
//             x = rand() % NUM_TILES_X;
//             y = rand() % NUM_TILES_Y;

//         } while (check if mines has been placed(x, y))
//         // put all the mines
//     }
// }