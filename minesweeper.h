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

void find_adjacent();

void update_adj_count(int i, int j);

int cal_adjacent_mines(int x, int y);

char *print_game();

char *print_fullGameBroad();

void place_mines();

int tile_contains_mine(int x, int y);

void placeFlag(char tileA, char tile1);

void revealTile(char tileA, char tile1);

void reveal_adj(int i, int j);

void reveal_ZeroAdj(int x, int y);

int getResult();

int getStatus();

int getTime();
