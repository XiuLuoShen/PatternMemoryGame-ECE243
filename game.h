#ifndef GAME_H_   /* Include guard */
#define GAME_H_

typedef struct {
    // the board containing the pattern, 1 means the tile is part of the pattern
    int** board;
    /* the board containing the selected tiles
     * 0 means unselected, 1 means correctly selected, 2 means wrongly selected
     */
    int** selectedTiles;
    unsigned boardSize;  // the size of the board
    unsigned numOfTiles; // the number of tiles in the pattern
    unsigned tilesFound; // the number of tiles found in the pattern

    // The following data members may be unneccessary
    unsigned lives;  // lives left in this game
    unsigned level;  // current level
    unsigned wrongTiles;      // the number of wrong tiles
} Game;

extern Game* GAME;

/**
 * The result that occurs when you click the tile
 * STAY: stay on the level
 * PASS: go to next level
 * LOSE: you lost, go back to beginning
 */
typedef enum{STAY, PASS, LOSE} ClickResult;

// Checks if the pattern was clicked
ClickResult checkBoard(void);

// Selects a tile
void selectTile(int row, int col);

void restartGame(void);

void startLevel(unsigned level);


// Initialize the game board
void initializeBoard(unsigned boardSize, unsigned numOfTiles);

// Free the memory used by the board
void freeBoard(void);

#endif // GAME_H_
