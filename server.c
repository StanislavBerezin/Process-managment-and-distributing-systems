
// imports
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define AUTH_TXT "Authentication.txt"

#define RANDOM_NUMBER_SEED 42
#define MINES_NUMBER 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

// functions declarations
void init();
void exitGame();

// Main function

int main(int argc, char *argv[])
{
    signal(SIGINT, exitGame);

    return 1;
}

// Function definitions

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