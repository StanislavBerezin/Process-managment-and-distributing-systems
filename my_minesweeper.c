#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <time.h>
#include <string.h>


#define RANDOM_NUMBER_SEED 42
#define NUM_TILES_X 9
#define NUM_TILES_Y 9
#define NUM_MINES 10

typedef struct {
    int adjacent_mines;
    bool revealed;
    bool is_mine;
} Tile;

typedef struct {
    // additional fields
    Tile tiles[NUM_TILES_X] [NUM_TILES_Y];
} GameState;

// enum {
//     R = 1,
//     P = 2,
//     Q = 3,
//     N = 0
// } option;

GameState playerState;

int remainMines, user_insertX, user_insertY;
char Column_letter;
bool gameover = false;

// Function for declarion

// Setting up the Game 
void gameSetUp();
void place_mines();
int cal_adjacent_mines();
void displayTheGoalBroad();
void displayGameBroad();

//Check the game status
void showAllMines();                          // Reveal all the mines and print it to terminal
int tile_contains_mine(int x, int y);         // Check the position, is it contain mine

// Selection / Movement Set
void selectOption();
void revealTile(int x, int y);
void PlaceFlag(int x, int y);

void insertCoordinate();
void revealSurroundingZeroTiles(int x, int y);

void win();
void gameOver();


int main(int argc, char * argv[]){
    srand(RANDOM_NUMBER_SEED);
    gameSetUp();
    while(gameover == false){
        displayGameBroad();
        selectOption();
    }
    return 0;
}

// ****************** Function for set up the game ****************************
void gameSetUp(){
    place_mines();
    for (int y = 0; y < NUM_TILES_Y; y++){
        for (int x = 0; x < NUM_TILES_X; x++){
            playerState.tiles[x][y].adjacent_mines = cal_adjacent_mines(x,y);
        }
    }
}

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

void displayTheGoalBroad(){

    printf("Remaining mines: %d \n", remainMines);
    puts("     1  2  3  4  5  6  7  8  9  ");
    puts("--------------------------------");
    char line[] = "A | ";

    for(int y = 0; y < NUM_TILES_Y; y++ ){
        // Print A B C D E
        printf("%s", line);

        for (int x = 0; x < NUM_TILES_X; x++){

            if (tile_contains_mine(x,y)){
                // if it is a bomb, show it as *
                printf(" * ");
            } else {
                // else print the adjacent mines number
                printf(" %d ", playerState.tiles[x][y].adjacent_mines);
            }

        } // End of the horizontial line loop x
        printf("\n");
        line[0] += 1;
    } // End of the Vertical line loop y
}

// Display current game status to the user
void displayGameBroad(){

    printf("Remaining mines: %d \n", remainMines);
    puts("     1  2  3  4  5  6  7  8  9  ");
    puts("--------------------------------");
    char line[] = "A | ";

    for(int y = 0; y < NUM_TILES_Y; y++ ){
        // Print A B C D E
        printf("%s", line);

        for (int x = 0; x < NUM_TILES_X; x++){
            
            // If the tiles already revealed
            if (playerState.tiles[x][y].revealed == true){
                if ( tile_contains_mine(x,y) ){   // If this tile is a bomb
                    printf(" + ");
                } else {                        // else print the adjacent mines number
                    printf(" %d ", playerState.tiles[x][y].adjacent_mines);
                }
            }

            else { // The tiles is not revealed by the user
                printf(" - ");
            }

        } // End of the horizontial line loop x
        printf("\n");

        line[0] += 1;
    } // End of the Vertical line loop y

}

// Show all the mines in the terminal
void showAllMines(){
    for(int x = 0; x < NUM_TILES_X; x++){
        for (int y= 0 ; y < NUM_TILES_Y; y++){
            if (tile_contains_mine(x,y)){
                char a = 'A';
                printf("tiles %d%c must be mine ! %d \n\n ", x + 1 , a + y , playerState.tiles[x][y].is_mine);
            }
        }
    }
}

void selectOption(){

    char option;

    while( (option != 'R') && (option != 'P') && (option != 'Q') ) {
        printf("\n\n");
        printf("choose an options:  \n");
        printf("<R> Reveal title \n");
        printf("<P> Place flag \n");
        printf("<Q> Quit game \n");
        printf("------------------ \n");
        printf("Option (R,P,Q)  \n");
        scanf("%c", &option);
    }

    if( option == 'Q'){
        printf("Bye");
        exit(1);
    }

    insertCoordinate();

    if ( option == 'R' ){
        revealTile(user_insertX, user_insertY);
    } else if ( option == 'P'){
        PlaceFlag(user_insertX, user_insertY);
    }

}

void insertCoordinate(){

    bool isValid = false;

    while (isValid == false){
        printf("Please enter selection:\n");
        printf("Row Number 1 - 9 : \n ");
        scanf("%d", &user_insertX);

        printf("Col Number A - I : \n ");
        scanf(" %c", &Column_letter);

        isValid = true;

        if (user_insertX > 9 || user_insertX < 1){
            printf("The Row number is between 1 - 9 only. please try again. \n\n");
            isValid = false;
        }
        else {
            // Because array index is minimum 0 - maximum 8 
            user_insertX--;
        }

        switch(Column_letter){
            case 'A':
                user_insertY = 0;
                break;
            case 'B':
                user_insertY = 1;
                break;
            case 'C':
                user_insertY = 2;
                break;
            case 'D':
                user_insertY = 3;
                break;
            case 'E':
                user_insertY = 4;
                break;
            case 'F':
                user_insertY = 5;
                break;
            case 'G':
                user_insertY = 6;
                break;
            case 'H':
                user_insertY = 7;
                break;
            case 'I':
                user_insertY = 8;
                break;
            default:
                printf("Col Number start from A to I. Please insert again \n");
                isValid = false;
                break;
            }
    }

    printf("You Entered Row : %d Col : %c \n\n", user_insertX, Column_letter);
}

void revealTile(int x , int y){

    if ( playerState.tiles[x][y].is_mine ){
        printf("You lose !!! \n");
        displayTheGoalBroad();
        gameover = true;
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

void PlaceFlag(int x, int y){
    if (playerState.tiles[x][y].is_mine == true ){
        playerState.tiles[x][y].revealed = true;
        remainMines--;
    } else {
        printf("Your can not place flag there. \n\n");
    }
}

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

int tile_contains_mine(int x, int y){
    if (playerState.tiles[x][y].is_mine == true){
        return 1;
    }
    return 0;
}
