 /* CAB403 System Programming Assignment 1
    Implemented By:
        KA LONG Lee (Eric) (N9845097)
        Stanislav Berezin (N9694315)
*/


 #define RANDOM_NUMBER_SEED 42

 #define NUM_TILES_X 9
 #define NUM_TILES_Y 9
 #define NUM_MINES 10

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct{
 	int adj_mines;
 	bool revealed;
 	bool is_mine;
 	bool is_flagged;
 } Tile;

typedef struct{
	Tile tiles[NUM_TILES_X][NUM_TILES_Y];
} GameState;

// Main function to run the game
int mainGame();

void cleanGame();

// A function to check this coordinate is mine or not
int tile_contains_mine(int x, int y);

// A function to place the mines at random position
void place_mines();

// A function that generate a string that can display the full game Broad (include all the information for all coordinate)
char *print_fullGameBroad();

// A function that create string to display a game broad to the user (according to the current game status)
char *print_game();

// A function to calculate the adj mines faster, if this position is mines then update the grid adj mines number 
void update_adj_count(int i, int j);

// A function that counting the adj number for this coordinate
int cal_adjacent_mines(int x, int y);

// a function that detect a mine and then update the adj tile count for the coordinate nearby
void reveal_adj(int i, int j);

// it is a function to support user reveal the zero tile, it will reveal the other nearby this zero tile
void reveal_ZeroAdj(int x, int y);

// a function that detect a mine and then update the adj tile count for the coordinate nearby
void find_adjacent();

// A function to decide the decision what should do if the user reveal the tile
void revealTile(char tileA, char tile1);

// A function that place a flag to the position
void placeFlag(char tileA, char tile1);

//Check the user , is it won the game
int getResult();

// If the user lose return the time to the user
int getStatus();

//  Return the time to the user
int getTime();
