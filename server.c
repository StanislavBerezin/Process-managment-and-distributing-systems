

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

#define NUM_MINES 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

// VARIABLES FOR BOOTING UP THE GAME
int hour, minute, second;
bool game_on = true;
bool successAuth = false;

pthread_mutex_t write_leader_mutex, read_leader_mutex;

// *************  Connection Variable ***********************
int PORT_NUM = 12345;
int client_socket, server_socket;
int user_list_count; //Counter for the AuthTxt file
struct sockaddr_in client_address, server_address;
int loggedIn;

// Login Details
struct User
{
    char *username;
    char *password;
} * users;

// **********************************************************


// ******************  Game  Variable ***********************
typedef struct {
    int adjacent_mines;
    bool revealed;
    bool is_mine;
} Tile;

typedef struct {
    // additional fields
    Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
} GameState;

GameState playerState;

int remainMines, user_insertX, user_insertY;
char Column_letter;

// **********************************************************

//Process
pid_t childpid;

// Variable
char buffer[MAXBUFFERSIZE];

// functions declarations
// BOOTING UP THE GAME
void prepareApp();
void initialiseTimer();
void read_AUTH_File();
void exitProgram();
void createMutexes();

// Connection Function
void buildTheApp(int argc, char *argv[]);
void setPort(int argc, char *argv[]);
void serverSetUP();
void awaitNewUsers();
void ReceiveMsgFromClient();
void sendMsgToClient(char *msg);
void CheckLoginDetails();

// Game function

// Setting up the Game 
void gameSetUp();
void place_mines();
int  cal_adjacent_mines();
void cal_adjacent_mines_for_all_tiles();

// Send game status to Client
void waitClientSelectMenu();
void SendGameBroadToClient();
void SendGoalBroadToClient();

//Check the game status
void showAllMines();                          // Reveal all the mines and print it to terminal
int tile_contains_mine(int x, int y);         // Check the position, is it contain mine

// Selection / Movement Set
void selectOption();
void revealTile(int x, int y);
void PlaceFlag(int x, int y);
void insertCoordinate();

void revealSurroundingZeroTiles(int x, int y);


// EXITING THE GAME
void exitProgram();


// Main function

int main(int argc, char *argv[])
{
    system("clear");
    prepareApp();
    buildTheApp(argc, argv);
    // After here the user has already login successfully 
    gameSetUp();

    waitClientSelectMenu();

    SendGameBroadToClient();
    SendGoalBroadToClient();
    // displayTheGoalBroad();
    // while (game_on)
    // {
    // }

    return 1;
}

void prepareApp()
{
    srand(RANDOM_NUMBER_SEED);
    signal(SIGINT, exitProgram);
    // time works, but disabled for further development
    // initialiseTimer();
    createMutexes();
    read_AUTH_File();
}

void buildTheApp(int argc, char *argv[])
{
    setPort(argc, argv);
    // Build up the server program, and listening to the port, waiting for client connection.
    serverSetUP();
    // Waiting for user to send the login information back
    awaitNewUsers();
}


void ReceiveMsgFromClient()
{
    // If we can not receive msg from the server
    if (recv(client_socket, buffer, MAXBUFFERSIZE, 0) < 0)
    {
        printf("[-] Error in receiving data. \n");
    } else
    {
        printf("From client : %s \n", buffer);
    }

    // We need to turn off the game when client send a exit packet
    // That is why last time it keep saying From client %s
    if (strcmp(buffer, "exit") == 0 ){
        printf(" Client Terminated the game... \n");
        game_on = false;
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
        printf("Error: Server Socket can not be made \n");
        exit(1);
    }

    // Disable this code for temporary
    // memset(&server_address, '\0', sizeof(server_address));

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

        if(successAuth == false){
            sendMsgToClient("Login failed! Check password");
        }

    }
}

// Read File Function
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

void exitProgram()
{
    printf("\n\n Ctrl+c was pressed, caused an interupt, closing connection \n\n");

    // will need to free memory here
    close(server_socket);
    close(client_socket);

    printf("Memory has been freed, sockets and memory are down\n");
    exit(1);
}


// ********************************* function for Game ****************************************

// Place the mines to the game broad and init the adjacent number to the gamebroad
void gameSetUp(){
    place_mines();
    cal_adjacent_mines_for_all_tiles();
}

void waitClientSelectMenu(){
    bool selectedOption = false;
    while( selectedOption == false ){
        sendMsgToClient("Please enter a selection");
        sendMsgToClient("<1> Play Minesweeper ");
        sendMsgToClient("<2> Show Leaderbroad");
        sendMsgToClient("<3> Quit");
        sendMsgToClient("Selection option (1-3) :");
        ReceiveMsgFromClient();
        if( (buffer[0] == '1' || buffer[0] == '2' || buffer[0] == '3' ) && (strlen(buffer) == 1) ){
            selectedOption = true;
        } else {
            sendMsgToClient("[-] Insert incorrect, only accept 1 - 3");
        }
    }
}

void SendGameBroadToClient(){

    sprintf(buffer,"Remaining mines :  %d ", remainMines);
    sendMsgToClient(buffer);

    sendMsgToClient("     1  2  3  4  5  6  7  8  9  ");
    sendMsgToClient("--------------------------------");

    for(int y = 0; y < NUM_TILES_Y; y ++){
        char column[40] = "";
        char line[] = "A | ";

        for (int x = 0; x < NUM_TILES_X; x++){

            // If the tiles already revealed
            if (playerState.tiles[x][y].revealed == true){
                if ( tile_contains_mine(x,y) ){   // If this tile is a bomb
                   strcat( column , " + ");
                } else {                        // else print the adjacent mines number
                    char adj_number[20];
                    sprintf(adj_number," %d ", playerState.tiles[x][y].adjacent_mines);
                    strcat(column, adj_number);
                }
            }

            else { // The tiles is not revealed by the user
               strcat( column , " - ");
            }

        }
        // Print Whole line
        line[0] += y;
        sendMsgToClient( strcat(line , column) );
    }

}

void SendGoalBroadToClient(){

    sprintf(buffer,"Remaining mines : %d ", remainMines);
    sendMsgToClient(buffer);

    sendMsgToClient("     1  2  3  4  5  6  7  8  9  ");
    sendMsgToClient("--------------------------------");

    for(int y = 0; y < NUM_TILES_Y; y ++){
        char column[40] = "";
        char line[] = "A | ";

        for (int x = 0; x < NUM_TILES_X; x++){

            if ( tile_contains_mine(x,y) ){   // If this tile is a bomb
                strcat( column , " * ");
            } else {                        // else print the adjacent mines number
                char adj_number[20];
                sprintf(adj_number," %d ", playerState.tiles[x][y].adjacent_mines);
                strcat(column, adj_number);
            }
        }
        // Print Whole line
        line[0] += y;
        sendMsgToClient( strcat(line , column) );
    }

}

// Randomly place the mines on the gamebroad
void place_mines(){
    remainMines = NUM_MINES;
    for (int i = 0; i < NUM_MINES; i++){
        int x, y;
        do {
            x = rand() % NUM_TILES_X;
            y = rand() % NUM_TILES_Y;
        } while (tile_contains_mine(x, y));
        playerState.tiles[x][y].is_mine = true;
    }
}

// Calculate the adjacent mines in this coordinate
int cal_adjacent_mines(int x, int y){
    int mines = 0;

    // Check up, down, left, right.
    if( (playerState.tiles[x - 1][y].is_mine == true) && (x - 1 != -1) )
        mines ++;

    if( (playerState.tiles[x + 1][y].is_mine == true) && (x + 1 <= NUM_TILES_X - 1) )
        mines ++;

    if( (playerState.tiles[x][y - 1].is_mine == true) && (y - 1 != -1) )
        mines ++;

    if( (playerState.tiles[x][y + 1].is_mine == true) && (y + 1 <= NUM_TILES_Y - 1) )
        mines ++;

    // Check all diagonal directions
    if( (playerState.tiles[x - 1][y + 1].is_mine == true) && (x - 1 != -1) && (y + 1 <= NUM_TILES_Y - 1) )
        mines ++;
    if( (playerState.tiles[x - 1][y - 1].is_mine == true) && (x - 1 != -1)  &&  (y - 1 != -1) )
        mines ++;
    if( (playerState.tiles[x + 1][y + 1].is_mine == true) && (x + 1 <= NUM_TILES_X - 1 ) && (y + 1 <= NUM_TILES_Y - 1) )
        mines ++;
    if( (playerState.tiles[x + 1][y - 1].is_mine == true) && (x + 1 <= NUM_TILES_X - 1 ) && (y - 1 != -1) )
        mines ++;

    return mines;
}

void cal_adjacent_mines_for_all_tiles(){
    // Calculate all the adjacent mines for all the tiles
    for (int y = 0; y < NUM_TILES_Y; y++){
        for (int x = 0; x < NUM_TILES_X; x++){
            playerState.tiles[x][y].adjacent_mines = cal_adjacent_mines(x,y);
        }
    }
}

// Reveal a tile according to the input coordinate
void revealTile(int x , int y){

    if ( playerState.tiles[x][y].is_mine ){
        printf("You lose !!! \n");
        // displayTheGoalBroad();
        game_on = false;
        exit(1);
    }

    if (playerState.tiles[x][y].revealed){
        printf("This tile has already revealed !, Please insert another tile. \n\n");
    } 
    
    else {
        playerState.tiles[x][y].revealed = true;
        // if this target is a zero, then check the adjacent mines is it zero
        if ( playerState.tiles[x][y].adjacent_mines == 0){
            revealSurroundingZeroTiles(x, y);
        }
    }

}

// Place a flag on the mines
void PlaceFlag(int x, int y){
    if (playerState.tiles[x][y].is_mine == true ){
        playerState.tiles[x][y].revealed = true;
        remainMines--;
    } else {
        printf("Your can not place flag there. \n\n");
    }
}

// If user found a Zero Tiles, it will reveal all the zero tiles nearby
void revealSurroundingZeroTiles(int x, int y){
    int nearByMines = 0;
    int i = 0;

    // Checking Up Position
    while( y - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x, y - i);
        if ( nearByMines == 0 ){
            playerState.tiles[x][y - i].revealed = true;
        }
        i++;
    }

    // Checking down Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x, y + i);
        if ( nearByMines == 0 ){
            playerState.tiles[x][y + i].revealed = true;
        }
        i++;
    }

    // Checking Left Position
    i = 0;
    nearByMines = 0;
    while( x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y);
        if ( nearByMines == 0 ){
            playerState.tiles[x - i][y].revealed = true;
        }
        i++;
    }

    // Checking Right Position
    i = 0;
    nearByMines = 0;
    while( x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y);
        if ( nearByMines == 0 ){
            playerState.tiles[x + i][y].revealed = true;
        }
        i++;
    }

    // Checking Down-Left Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y + i);
        if ( nearByMines == 0 ){
            playerState.tiles[x - i][y + i].revealed = true;
        }
        i++;
    }

    // Checking Down-Right Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y + i);
        if ( nearByMines == 0 ){
            playerState.tiles[x + i][y + i].revealed = true;
        }
        i++;
    }

    // Checking Up-Left Position
    i = 0;
    nearByMines = 0;
    while( y - i != -1 && x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y - i);
        if ( nearByMines == 0 ){
            playerState.tiles[x - i][y - i].revealed = true;
        }
        i++;
    }

    // Checking Up-Right Position
    i = 0;
    nearByMines = 0;
    while( y - i != -1 && x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y - i);
        if ( nearByMines == 0 ){
            playerState.tiles[x + i][y - i].revealed = true;
        }
        i++;
    }

    //End Checking

}

// Check this Coordinate is it mine or not
int tile_contains_mine(int x, int y){
    if (playerState.tiles[x][y].is_mine == true){
        return 1;
    }
    return 0;
}

// initialising Timer for the game
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