#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "minesweeper.h"
#include <sys/time.h>


struct timeval  tv1, tv2;


GameState game;
int remainingMines;

int tile_contains_mine(int x, int y){
	return game.tiles[x][y].is_mine;
}

void place_mines(){
	for(int i = 0; i < NUM_MINES;i++){
		int x,y;
		do{
			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;
		}while(tile_contains_mine(x,y));
		game.tiles[x][y].is_mine = true;
		game.tiles[x][y].is_flagged = false;
	}
}

char * print_fullGameBroad(){
	int x,y;
	char out[400];
	
	sprintf(out, "Remaining Mines = %d\n    ", remainingMines);
	for(x=1;x<=NUM_TILES_Y;x++) sprintf(out, "%s%d ", out, x);
	sprintf(out, "%s\n----", out);
	for(x=0;x<NUM_TILES_Y;x++) sprintf(out, "%s--", out);
	sprintf(out, "%s\n", out);

	for(x=0;x<NUM_TILES_X;x++){
		sprintf(out, "%s%c | " , out, (char)('A' + x));
			for(y=0;y<NUM_TILES_Y;y++){
				if(game.tiles[x][y].is_mine){
					sprintf(out, "%s* ", out);
				}
                else {
                    sprintf(out, "%s%d ", out, game.tiles[x][y].adj_mines);
                }
				if(NUM_TILES_Y > 9) sprintf(out, "%s ", out);
		}
		sprintf(out, "%s\n", out);
	}

	return strdup(out);
}

char * print_game(){
	int x,y;
	char out[400];
	
	sprintf(out, "\nRemaining Mines = %d\n    ", remainingMines);
	for(x=1;x<=NUM_TILES_Y;x++) sprintf(out, "%s%d ", out, x);
	sprintf(out, "%s\n----", out);
	for(x=0;x<NUM_TILES_Y;x++) sprintf(out, "%s--", out);
	sprintf(out, "%s\n", out);

	for(x=0;x<NUM_TILES_X;x++){
		sprintf(out, "%s%c | " , out, (char)('A' + x));
			for(y=0;y<NUM_TILES_Y;y++){
				if(game.tiles[x][y].is_flagged)
					sprintf(out, "%s+ ", out);
				else if(game.tiles[x][y].revealed){
					if(game.tiles[x][y].is_mine){
					sprintf(out, "%s* ", out);
					}else
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

void reveal_adj(int x, int y){

    int nearByMines = 0;
    int i = 0;

    // Checking Up Position
    while( y - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x, y - i);
        if ( nearByMines == 0 ){
            game.tiles[x][y - i].revealed = true;
        }
        i++;
    }

    // Checking down Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x, y + i);
        if ( nearByMines == 0 ){
            game.tiles[x][y + i].revealed = true;
        }
        i++;
    }

    // Checking Left Position
    i = 0;
    nearByMines = 0;
    while( x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y);
        if ( nearByMines == 0 ){
            game.tiles[x - i][y].revealed = true;
        }
        i++;
    }

    // Checking Right Position
    i = 0;
    nearByMines = 0;
    while( x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y);
        if ( nearByMines == 0 ){
            game.tiles[x + i][y].revealed = true;
        }
        i++;
    }

    // Checking Down-Left Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y + i);
        if ( nearByMines == 0 ){
            game.tiles[x - i][y + i].revealed = true;
        }
        i++;
    }

    // Checking Down-Right Position
    i = 0;
    nearByMines = 0;
    while( y + i != NUM_TILES_Y && x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y + i);
        if ( nearByMines == 0 ){
            game.tiles[x + i][y + i].revealed = true;
        }
        i++;
    }

    // Checking Up-Left Position
    i = 0;
    nearByMines = 0;
    while( y - i != -1 && x - i != -1 && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x - i, y - i);
        if ( nearByMines == 0 ){
            game.tiles[x - i][y - i].revealed = true;
        }
        i++;
    }

    // Checking Up-Right Position
    i = 0;
    nearByMines = 0;
    while( y - i != -1 && x + i != NUM_TILES_X && nearByMines == 0){
        nearByMines = cal_adjacent_mines(x + i, y - i);
        if ( nearByMines == 0 ){
            game.tiles[x + i][y - i].revealed = true;
        }
        i++;
    }
}

void find_adjacent(){
	int x,y;
	for(x=0;x<NUM_TILES_X;x++){
		for(y=0;y<NUM_TILES_X;y++){
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

int mainGame(){
    bzero(&game, sizeof(GameState));
 	flags_placed = 0;
	srand(RANDOM_NUMBER_SEED);
    gettimeofday(&tv1, NULL);
	place_mines();
	find_adjacent();
	gameover = false;
    remainingMines = 10;
    valid = true;
}

void revealTile(char tileA, char tile1)
{
    int row;
    int col;

    row = tileA - 'A';
	col = tile1 - '1';

	if(game.tiles[row][col].is_mine && !game.tiles[row][col].is_flagged){
		gameover = true;
		return;
	}

	// if this coordinate is zero, and it never be revealed before
	if(game.tiles[row][col].adj_mines == 0 && game.tiles[row][col].revealed == false){
		reveal_adj(row,col);
	}
    game.tiles[row][col].revealed = true;
	print_game();
	valid = true;
}

void placeFlag(char tileA, char tile1)
{
    int row;
    int col;

    row = tileA - 'A';
	col = tile1 - '1';

	game.tiles[row][col].is_flagged = true;
	flags_placed++;
	if (game.tiles[row][col].is_mine)
	{
		remainingMines--;
	}
	if (flags_placed == 10)
	{
        gameover = true;
	}
	print_game();
	valid = true;
}

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

int getTime()
{
    return  (tv2.tv_usec - tv1.tv_usec) / 1000000 +
          (tv2.tv_sec - tv1.tv_sec);
}
