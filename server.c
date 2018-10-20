

#define _GNU_SOURCE
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
#define MAX_PLAYERS 10
#define RANDOM_NUMBER_SEED 42
#define MINES_NUMBER 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

// VARIABLES FOR BOOTING UP THE GAME
int hour, minute, second;
bool game_on = true;
bool successAuth = false;

pthread_mutex_t write_leader_mutex, read_leader_mutex;

//Connection
int PORT_NUM = 12345;
int client_socket, server_socket;
int user_list_count; //Counter for the AuthTxt file
struct sockaddr_in client_address, server_address;
int loggedIn;

//Process
pid_t childpid;

// Variable
char buffer[MAXBUFFERSIZE];

// functions declarations
// BOOTING UP THE GAME
void prepareApp();
void initialiseTimer();
void read_AUTH_File();
void exitGame();
void createMutexes();

// CREATING GAME AND GAME FUNCTIONALITY
void buildTheApp(int argc, char *argv[]);
void setPort(int argc, char *argv[]);
void serverSetUP();
void awaitNewUsers();
void ReceiveMsgFromClient();
void sendMsgToClient(char *msg);
void CheckLoginDetails();

// EXITING THE GAME
void exitGame();

struct User
{
    char *username;
    char *password;
} * users;

// Main function

int main(int argc, char *argv[])
{
    system("clear");
    prepareApp();
    buildTheApp(argc, argv);

    return 1;
}

void prepareApp()
{
    srand(RANDOM_NUMBER_SEED);
    signal(SIGINT, exitGame);
    // time works, but disabled for further development
    // initialiseTimer();
    createMutexes();
    read_AUTH_File();
}

void buildTheApp(int argc, char *argv[])
{
    setPort(argc, argv);
    serverSetUP();
    awaitNewUsers();
    while (game_on)
    {

        ReceiveMsgFromClient();
    }
}
void ReceiveMsgFromClient()
{
    // If we can not receive msg from the server
    if (recv(client_socket, buffer, MAXBUFFERSIZE, 0) < 0)
    {
        printf("[-] Error in receiving data. \n");
    }
    else
    {
        printf("From client : %s \n", buffer);
    }
}
// Function for sending msg to the client
void sendMsgToClient(char *msg)
{
    strcpy(buffer, msg);
    // Send msg to the network socket
    // You only need to change the value of buffer outside
    if (send(client_socket, buffer, MAXBUFFERSIZE, 0) == -1)
    {
        printf("[-] Error in sending data. \n");
    }
}

// Function definitions
// Send Message back to Client
void serverSetUP()
{

    // Server Socket init
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    // Check Socket Connection
    if (server_socket < 0)
    {
        printf("Error: bad connection \n");
        exit(1);
    }

    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    // basically will result to any IP address on the local machine
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to our specified IP and port
    int ret = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (ret < 0)
    {
        printf("Error: Failed to bind specified IP/port \n");
        exit(1);
    }
    // *** Important *** second argument is a backlog which is
    // how many connection can be waiting for this particular socket at one point in time
    if (listen(server_socket, MAX_PLAYERS) < 0)
    {
        printf("Waiting for players on %d.... \n", PORT_NUM);
    }
    puts("Waiting for players...");
}

void CheckLoginDetails()
{

    while (!successAuth)
    {
        ReceiveMsgFromClient();

        char *username = strtok(buffer, " ");
        char *password = strtok(NULL, "");

        for (int i = 1; i < user_list_count; i++)
        {
            // If username match one of the user on the list
            if (strcmp(users[i].username, username) == 0)
            {
                // If password match
                if (strcmp(users[i].password, password) == 0)
                {
                    sendMsgToClient("Welcome to minesweeper");

                    successAuth = true;
                }
            }
        }

        sendMsgToClient("Login failed! Check username and password");
    }
}

void awaitNewUsers()
{

    // Notes: It is just a testing function, will change later on
    socklen_t addr_size;
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &addr_size);

    // Might need to change something, always failed here
    if (client_socket < 0)
    {
        printf("Error: Client Connection Failed ");
        exit(1);
    }
    else
    {
        printf("Connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        successAuth = false;
        CheckLoginDetails();
    }
}

void read_AUTH_File()
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
void initialiseTimer()
{

    hour = minute = second = 0;
    while (1)
    {
        system("clear");
        //clear output screen
        //print time in HH : MM : SS format
        // SHOULD BE USED WHENEVER WE NEED IT TO DISPLAY AFTER
        printf("%02d : %02d : %02d ", hour, minute, second);
        //clear output buffer in gcc
        fflush(stdout);
        //increase second
        second++;
        //update hour, minute and second
        if (second == 60)
        {
            minute += 1;
            second = 0;
        }
        if (minute == 60)
        {
            hour += 1;
            minute = 0;
        }
        if (hour == 24)
        {
            hour = 0;
            minute = 0;
            second = 0;
        }
        sleep(1); //wait till 1 second
    }
}

// Setting up the port
void setPort(int argc, char *argv[])
{
    if (argc == 1)
    {
        puts("\n");
        printf("Setting up default port on: %d", PORT_NUM);
        puts("\n");
    }
    else if (argc == 2)
    {
        int length = strlen(argv[1]);

        if (length > 5)
        {
            puts("\n");
            printf("Error: Try port number with 5 charecters");
            puts("\n");
        }
        else
        {
            PORT_NUM = atoi(argv[1]);
            puts("\n");
            printf("Setting port on port: %d", PORT_NUM);
            puts("\n");
        }
    }
    else
    {
        puts("\n");
        printf("Error: Too many arguments, try again");
        puts("\n");
    }
}
// This is required for accessing leaderboard, for example if somebody
// have won the game, it will need to block anyone from reading it, while
// updating it. USED FOR CRITICAL SECTION
// More info: https://stackoverflow.com/questions/14888027/mutex-lock-threads
void createMutexes()
{
    pthread_mutex_init(&write_leader_mutex, NULL);
    pthread_mutex_init(&read_leader_mutex, NULL);
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
