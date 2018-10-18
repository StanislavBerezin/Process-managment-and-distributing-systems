
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
struct sockaddr_in client_address, server_address;

//Process
pid_t childpid;

// Variable
char buffer[MAXBUFFERSIZE];

// functions declarations
void init();
void exitGame();
void ServerSetUP();
int CheckLoginDetails();
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

    ServerSetUP();

    while(CheckLoginDetails()){
        
    }


    // if ( (childpid = fork() ) == 0){
    //     close(server_socket);
    //     CheckLoginDetails();
    // }
    free(buffer);
    // Close the socket connection
    close(server_socket);
    return 1;
}

int CheckLoginDetails(){

    // To Stas: It is almost finished.
    // I don't know why sometime it didn't quit the while loop
    // It also happens on Client Side

    int loggedIn = 0;
    
    // Something Stupid wrong with the While loop
    while (loggedIn == 0){
        recv(client_socket, buffer, MAXBUFFERSIZE, 0);
        printf("Client Said UserName is %s \n", buffer);

        if(strcmp(buffer, "Eric 123456") == 0){
            strcpy(buffer, "login success");
            printf("[+] Login Success ! \n ");
            send(client_socket, buffer, sizeof(buffer), 0);
            bzero(buffer, sizeof(buffer));
            return 0;
        } else {
            strcpy(buffer, "login Fail \n");

            send(client_socket, buffer, sizeof(buffer), 0);
            bzero(buffer, sizeof(buffer));
            return 1;
        }
    }
    
}

// Function definitions
// Send Message back to Client
void ServerSetUP(){
    // Notes: It is just a testing function, will change later on

    socklen_t addr_size;

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
    int ret = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

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

    client_socket = accept(server_socket, (struct sockaddr *) &client_address, &addr_size );

    // Might need to change something, always failed here
    if (client_socket < 0){
        printf("[-] Client Connection Failed ");
        exit(1);
    }

    printf("[+]Connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port) );
    
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