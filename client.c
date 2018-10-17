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
#define h_addr h_addr_list[0]

char username[NAME_LENGTH];
char password[PASS_LENGTH];

// functions declarations
void welcomeToGameScreen();
void authenticate();
void connectToServer();
// Main function

//Connection Variable
int PORT_NUM;
struct hostent *he;

int main(int argc, char *argv[])
{
    // Need to improve this one
    if ((he = gethostbyname(argv[1])) == NULL)
    { /* get the host info */
        printf("[-]Please insert IP address");
        exit(1);
    }

    if (argc != 3)
    {
        printf("[-]Usage: server port_number");
        exit(1);
    }

    // Read PORT Number from argv
    PORT_NUM = atoi(argv[2]);

    welcomeToGameScreen();
    connectToServer();
    return 1;
}
// need to send details to server from char username and password
void authentate()
{
}
// connect to server
void connectToServer()
{
    // Notes: Simple demo for testing Connection, will change later on

    // Create a socket
    int network_socket;
    // First Parameter is the domain of the socket -> Becuase it is internet socket so it is AF_INET
    // Second Parameter SOCK_STEAM = Using TCP
    // thrid Parameter is define the protocol we use 0 for TCP
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Specify an address for the socket
    struct sockaddr_in server_address;
    // Becuase it is internet socket so it is AF_INET
    server_address.sin_family = AF_INET;
    // Using 9002 Por
    server_address.sin_port = htons(PORT_NUM);
    // Connect to 0.0.0.0 IP
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

    char msgSend[1024];

    while(1){
        printf("Client: \t");
        // Read InPut from Client side user
        scanf("%s", &msgSend[0]);
        // Send msg to the network socket
        // socket/ msg / size / flag
        send(network_socket, msgSend, strlen(msgSend), 0);

        // If user enter :exit
        if(strcmp(msgSend, ":exit") == 0){
            // Close the socket port
            close(network_socket);
            printf("[-] Disconnected from server. \n");
            // End Program
            exit(1);
        }

        // If we can not receive msg from the server
        if( recv(network_socket, msgSend, 1024 , 0) < 0 ){
            printf("[-] Error in receiving data. \n");
        } else {
            printf("Server : \t%s\n", msgSend);
        }
    }

    // // Recieve data from the server
    // char server_response[256];
    // // Socket / Pointer to the recieve data / size for the response / FLAG
    // recv(network_socket, &server_response, sizeof(server_response), 0);

    // // Print out the server 's response
    // printf(" %s \n", server_response);

    // // and then close the socket
    // close(network_socket);
}

// Function definitions
void welcomeToGameScreen()
{
    puts("=======================================================================\n");
    puts("Welcome to the online minesweeper gaming system.\n ");
    puts("=======================================================================\n \n");
    puts("You are required to login with your registered username and password to play");
    printf("Enter your username: ");
    scanf("%s", username);
    printf("Enter your password: ");
    scanf("%s", password);
}