#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "minesweeper.h"
#include <sys/time.h>

// Time structure
struct timeval  tv1, tv2;

int gameRound = 0;

GameState game;
int remainingMines;

// A function to check this coordinate is mine or not
int tile_contains_mine(int x, int y){
	return game.tiles[x][y].is_mine;
}

// A function to place the mines at random position
void place_mines(){
    // we should place ten mines on every gamebroad
	for(int i = 0; i < NUM_MINES;i++){
        // x y position init
		int x,y;
		do{
            // Put random number to the x y position between 0 - 8
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		}while(tile_contains_mine(x,y));
        // if the position is not mine, then we place the mines at this position
		game.tiles[x][y].is_mine = true;
        // in the very beginning the flag is not placed by user so this it none
		game.tiles[x][y].is_flagged = false;
	}
}

// A function that generate a string that can display the full game Broad (include all the information for all coordinate)
char * print_fullGameBroad(){
    // init a x y position detector
	int x,y;
    // Msg buffer
	char out[400];
	// Generate Remaining Mines Msg
	sprintf(out, "Remaining Mines = %d\n    ", remainingMines);
	for(x=1;x<=NUM_TILES_Y;x++) sprintf(out, "%s%d ", out, x);
    // Generate a line to seperate the remain msg and game broad
	sprintf(out, "%s\n----", out);
	for(x=0;x<NUM_TILES_Y;x++) sprintf(out, "%s--", out);
	sprintf(out, "%s\n", out);

	for(x=0;x<NUM_TILES_X;x++){
		sprintf(out, "%s%c | " , out, (char)('A' + x));
			for(y=0;y<NUM_TILES_Y;y++){
				if(game.tiles[x][y].is_mine){
                    // if this coordinate is mine, use * to denoted as mines for user
					sprintf(out, "%s* ", out);
				}
                else {
                    // if this coordinate is not mine, we should show the adj mines for this coordinate
                    sprintf(out, "%s%d ", out, game.tiles[x][y].adj_mines);
                }
				if(NUM_TILES_Y > 9) sprintf(out, "%s ", out);
		}
		sprintf(out, "%s\n", out);
	}

	return strdup(out);
}

// A function that create string to display a game broad to the user (according to the current game status)
char * print_game(){
    // init a x y position detector
	int x,y;
    // Msg buffer
	char out[400];

	// Generate Remaining Mines Msg
	sprintf(out, "\nRemaining Mines = %d\n    ", remainingMines);
	for(x=1;x<=NUM_TILES_Y;x++) sprintf(out, "%s%d ", out, x);
    // Generate a line to seperate the remain msg and game broad
	sprintf(out, "%s\n----", out);
	for(x=0;x<NUM_TILES_Y;x++) sprintf(out, "%s--", out);
	sprintf(out, "%s\n", out);

	for(x=0;x<NUM_TILES_X;x++){
		sprintf(out, "%s%c | " , out, (char)('A' + x));
			for(y=0;y<NUM_TILES_Y;y++){
				if(game.tiles[x][y].is_flagged)
					sprintf(out, "%s+ ", out); // if user place a flag at this position, use + to denoted as flag
				else if(game.tiles[x][y].revealed){
					if(game.tiles[x][y].is_mine){
                    // if user reveal this coordinate, and it is a mines it will denoted as *
					sprintf(out, "%s* ", out);
					}else
                    // If user reveal this coordinate, and it is not a mines will display the adj mines to this coordinate
					sprintf(out, "%s%d ", out, game.tiles[x][y].adj_mines);
				}
				else{
					sprintf(out, "%s  ", out);
				}
				if(NUM_TILES_Y > 9) sprintf(out, "%s ", out);
		}
		sprintf(out, "%s\n", out);
	}

	return strdup(out);
}

// A function to calculate the adj mines faster, if this position is mines then update the grid adj mines number 
void update_adj_count(int x, int y){
	if(x != NUM_TILES_X-1 && y != NUM_TILES_Y-1) 
		game.tiles[x+1][y+1].adj_mines++;
	if(x != NUM_TILES_X-1) 					 
		game.tiles[x+1][y].adj_mines++;
	if(x != NUM_TILES_X-1 && y != 0) 			 
		game.tiles[x+1][y-1].adj_mines++;
	if(y != NUM_TILES_Y-1) 					 
		game.tiles[x][y+1].adj_mines++;
	if(y != 0)			 					 
		game.tiles[x][y-1].adj_mines++;
	if(x != 0) 			 					 
		game.tiles[x-1][y].adj_mines++;
	if(x != 0 && y != 0) 					 
		game.tiles[x-1][y-1].adj_mines++;
	if(x != 0 && y != NUM_TILES_Y-1)			 
		game.tiles[x-1][y+1].adj_mines++;

}

// A function that counting the adj number for this coordinate
int cal_adjacent_mines(int x, int y){
    int mines = 0;

    // Check up, down, left, right.
    if( (game.tiles[x - 1][y].is_mine == true) && (x - 1 != -1) )
        mines ++;

    if( (game.tiles[x + 1][y].is_mine == true) && (x + 1 <= NUM_TILES_X - 1) )
        mines ++;

    if( (game.tiles[x][y - 1].is_mine == true) && (y - 1 != -1) )
        mines ++;

    if( (game.tiles[x][y + 1].is_mine == true) && (y + 1 <= NUM_TILES_Y - 1) )
        mines ++;

    // Check all diagonal directions
    if( (game.tiles[x - 1][y + 1].is_mine == true) && (x - 1 != -1) && (y + 1 <= NUM_TILES_Y - 1) )
        mines ++;
    if( (game.tiles[x - 1][y - 1].is_mine == true) && (x - 1 != -1)  &&  (y - 1 != -1) )
        mines ++;
    if( (game.tiles[x + 1][y + 1].is_mine == true) && (x + 1 <= NUM_TILES_X - 1 ) && (y + 1 <= NUM_TILES_Y - 1) )
        mines ++;
    if( (game.tiles[x + 1][y - 1].is_mine == true) && (x + 1 <= NUM_TILES_X - 1 ) && (y - 1 != -1) )
        mines ++;

    return mines;
}

// it is a function to support user reveal the zero tile, it will reveal the other nearby this zero tile
void reveal_adj(int x, int y){

    // Reveal Up Position
	if(y != 0)			 					 
        game.tiles[x][y-1].revealed = true;

    // Reveal down Position
	if(y != NUM_TILES_Y-1) 					 
        game.tiles[x][y+1].revealed = true;

    // Reveal Left Position
	if(x != 0) 			 					 
        game.tiles[x-1][y].revealed = true;

    // Reveal Right Position
	if(x != NUM_TILES_X-1) 					 
        game.tiles[x+1][y].revealed = true;
    
    // Reveal Up-Left Position
	if(x != 0 && y != 0) 					 
        game.tiles[x-1][y-1].revealed = true;

    // Reveal UP-Right Position
	if(x != NUM_TILES_X-1 && y != 0) 			 
        game.tiles[x+1][y-1].revealed = true;

    // Reveal Down-Left Position
	if(x != 0 && y != NUM_TILES_Y-1)			 
        game.tiles[x-1][y+1].revealed = true;

    // Reveal Down-Right Position
    if(x != NUM_TILES_X-1 && y != NUM_TILES_Y-1) 
        game.tiles[x+1][y+1].revealed = true;

}

// If user reveal a zeroAdj, this function is helped to reveal the nearby zero tile until the next tile is not zero
void reveal_ZeroAdj(int x, int y){
    // local Variable for checking the tile should it reveal more on this position or not
    int nearByMines = 0;
    int i = 0;

    // Checking Up Position
    while( y - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x, y - i);
        if ( nearByMines == 0 ){
            reveal_adj(x, y -i);
        }
        i++;
    }

    // Checking down Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x, y + i);
        if ( nearByMines == 0 ){
            reveal_adj(x, y +i );
        }
        i++;
    }

    // Checking Left Position
    i = 0;
    nearByMines = 0;
    while( x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y);
        if ( nearByMines == 0 ){
            reveal_adj(x - i, y);
        }
        i++;
    }

    // Checking Right Position
    i = 0;
    nearByMines = 0;
    while( x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y);
        // game.tiles[x + i][y].revealed = true;
        if ( nearByMines == 0 ){
            reveal_adj(x + i, y);
        }
        i++;
    }

    // Checking Down-Left Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y + i);
        // game.tiles[x - i][y + i].revealed = true;
        if ( nearByMines == 0 ){
            reveal_adj(x - i, y + i);
        }
        i++;
    }

    // Checking Down-Right Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y + i);
        // game.tiles[x + i][y + i].revealed = true;
        if ( nearByMines == 0 ){
            reveal_adj(x + i, y + i );
        }
        i++;
    }

    // Checking Up-Left Position
    i = 0;
    nearByMines = 0;
    while( y - i != -1 && x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y - i);
        if ( nearByMines == 0 ){
            reveal_adj(x - i, y - i);
        }
        i++;
    }

    // Checking Up-Right Position
    i = 0;
    nearByMines = 0;
    while( y - i != -1 && x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y - i);
        // game.tiles[x + i][y - i].revealed = true;
        if ( nearByMines == 0 ){
            reveal_adj(x + i, y -i);
        }
        i++;
    }
}

// a function that detect a mine and then update the adj tile count for the coordinate nearby
void find_adjacent(){
	int x,y;
	for(x=0;x<NUM_TILES_X;x++){
		for(y=0;y<NUM_TILES_X;y++){
                // if it is a mine, then update the adj tile count for the coordinate nearby
				if(game.tiles[x][y].is_mine){
					update_adj_count(x,y);
				}
		}
	}
}

bool gameover = false;
bool valid = true;
time_t t;
int flags_placed = 0;
double cpu_time_used;

int mainGame(int seed){
    bzero(&game, sizeof(GameState));
 	flags_placed = 0;
    srand(RANDOM_NUMBER_SEED + gameRound);
    gameRound++;
    // Start to record the time
    gettimeofday(&tv1, NULL);
    // Start to place the mines to gamebroad
	place_mines();
    // Start to Calculate the adj for all the tiles according to the bomb position
	find_adjacent();
	gameover = false;
    remainingMines = 10;
    valid = true;
}

// A function to decide the decision what should do if the user reveal the tile
void revealTile(char tileA, char tile1)
{
    int row;
    int col;

    row = tileA - 'A';
	col = tile1 - '1';

    // if this coordinate is a mine, and user is not place a flag , they are reveals it
	if(game.tiles[row][col].is_mine && !game.tiles[row][col].is_flagged){
		gameover = true;
		return;
	}

	// if this coordinate is zero, and it never be revealed before
	if(game.tiles[row][col].adj_mines == 0 && game.tiles[row][col].revealed == false){
        // Reveal the zero tile nearby from all position until it is not zero
		reveal_ZeroAdj(row,col);
	}
    game.tiles[row][col].revealed = true;
    // Print the game broad
	print_game();
	valid = true;
}

// A function that place a flag to the position
void placeFlag(char tileA, char tile1)
{
    int row;
    int col;

    // calculate the correct row and col 
    row = tileA - 'A';
	col = tile1 - '1';

    // Turn the is_flagged variable on
	game.tiles[row][col].is_flagged = true;
	flags_placed++;
    // If user placed on a correct position
	if (game.tiles[row][col].is_mine)
	{
        //Remaining Mines should minus one to tell the user, they are correct
		remainingMines--;
	}

    // Currently we are allowed the user place the flag on all the coordinate
    // We don't know the limitation, if yes we can change this number
	if (flags_placed == 81)
	{
        // misplaced flag lead to fail the game
        gameover = true;
	}
    // Print the game broad
	print_game();
	valid = true;
}

//If the user has won
int getResult()
{
    int x,y;
	for(x=0;x<NUM_TILES_X;x++)
	{
        for(y=0;y<NUM_TILES_X;y++)
        {
            if(game.tiles[x][y].is_mine && !game.tiles[x][y].is_flagged)
            {
                return 0;
            }
		}
	}
	return 1;
}

// If the user lose return the time to the user
int getStatus()
{
    if(gameover)
    {
		gettimeofday(&tv2, NULL);
        return 1;
    }
    else
        return 0;
}

//  Return the time to the user
int getTime()
{
    return  (tv2.tv_usec - tv1.tv_usec) / 1000000 +
          (tv2.tv_sec - tv1.tv_sec);
}
