#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define NAME_LENGTH 7
#define PASS_LENGTH 6

char username[NAME_LENGTH];
char password[PASS_LENGTH];

// functions declarations
void welcomeToGameScreen();
void authenticate();
void connectToServer();
// Main function

int main(int argc, char *argv[])
{
    welcomeToGameScreen();
    return 1;
}
// need to send details to server from char username and password
void authentate()
{
}
// connect to server
void connectToServer()
{
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