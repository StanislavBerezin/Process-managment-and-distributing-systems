
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

#define AUTH_TXT "Authentication.txt"

#define RANDOM_NUMBER_SEED 42
#define MINES_NUMBER 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

//Connection
#define PORT_NUMBER 9002

// functions declarations
void init();
void exitGame();

// Main function

int main(int argc, char *argv[])
{
    signal(SIGINT, exitGame);
    SendWelcomeMsg();
    return 1;
}

// Function definitions
// Send Message back to Client
void SendWelcomeMsg(){
    // Notes: It is just a testing function, will change later on

    // create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    // basically will result to any IP address on the local machine
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to our specified IP and port 
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    printf("start listening to the client!!! \n");
    // *** Important *** second argument is a backlog which is 
    // how many connection can be waiting for this particular socket at one point in time
    listen(server_socket, 5);
    
    // Get the socket
    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);
    printf("Package get from the client!!! \n");

    // Send the message
    char server_message[255] = "Welcome to Minesweeper in C! ";
    send(client_socket, server_message, sizeof(server_message), 0);

    printf("Close the Socket \n");
    
    // Close the socket
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