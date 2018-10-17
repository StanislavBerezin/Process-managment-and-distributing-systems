
// imports
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

//Sokcet Header file
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h> 
#include <errno.h> 

#define AUTH_TXT "Authentication.txt"

#define RANDOM_NUMBER_SEED 42
#define MINES_NUMBER 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

//Connection
int PORT_NUM;

// functions declarations
void init();
void exitGame();
void SendWelcomeMsg();

// Main function

int main(int argc, char *argv[])
{
    // Save PORT NUM from terminal argc
    if (argc != 2){
		printf("[-] Usage: server port_number \n");
		exit(1);
	}
    // ASCII to Int for PORT Number
	PORT_NUM = atoi(argv[1]);

    signal(SIGINT, exitGame);

    SendWelcomeMsg();
    return 1;
}

// Function definitions
// Send Message back to Client
void SendWelcomeMsg(){
    // Notes: It is just a testing function, will change later on

    // create the server socket
    int server_socket, ret;
    //define the server address
    struct sockaddr_in server_address;

    int client_socket;
    struct sockaddr_in client_address;

    socklen_t addr_size;

    char buffer[1024];
    pid_t childpid;

    // Server Socket init
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    // Check Socket Connection
    if (server_socket < 0){
        printf("[-]Error in connection \n");
        exit(1);
    }

    printf("[+]Server Socket is created. \n");

    memset(&server_address, '\0', sizeof(server_address) );
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    // basically will result to any IP address on the local machine
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to our specified IP and port 
    ret = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    if( ret <0 ){
        printf("[-]Fail to bind to specified IP and port \n");
        exit(1);
    }

    printf("[+]Bind successfully to port %d \n ", PORT_NUM);

    printf("[+]Start listening to the client!!! \n");
    // *** Important *** second argument is a backlog which is 
    // how many connection can be waiting for this particular socket at one point in time
    if (listen(server_socket, 10) == 0){
        printf("[+]Listening.... \n");
    } else {
        printf("[-]Error in binding. \n");
        exit(1);
    }

    while(1){
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &addr_size );

        if (client_socket < 0){
            exit(1);
        }

        printf("[+]Connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port) );

        if ( (childpid = fork() ) == 0){
            close(server_socket);

            while(1){
                recv(client_socket, buffer, 1024, 0);
                if (strcmp(buffer, ":exit") == 0 ){
                    printf("[-]Disconnected from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port) );
                    break;
                } else {
                    printf("Client : %s \n", buffer);
                    send(client_socket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                }

            }
        }
    }

    // // Get the socket
    // client_socket = accept(server_socket, NULL, NULL);
    // printf("Package get from the client!!! \n");

    // // Send the message
    // char server_message[255] = "Welcome to Minesweeper in C! ";
    // send(client_socket, server_message, sizeof(server_message), 0);

    // printf("Close the Socket \n");
    
    // Close the socket connection
    close(server_socket);
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