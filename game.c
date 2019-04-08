#include <stdlib.h>

#include "game.h"

bool playerTurn;

void restartGame(void) {
    // Free the board
    GAME->timesPlayedOnThisSize = 1;
    GAME->level = 1;
    GAME->lives = 3;
    GAME->wrongTiles = 0;
    GAME->currentTileX = 0;
    GAME->currentTileY = 0;
    lost = false;
    // freeBoard();

    // Start game with board size 3x3 and 3 tiles
    initializeBoard(3, 3);
    playerTurn = false;
}

void newLevel(unsigned level) {
    // Free the board
    freeBoard();
    // If level isn't being repeated, increment level and number of tiles to remember
    if (GAME->level < level) {
        GAME->level++;
        GAME->numOfTiles++;
        GAME->timesPlayedOnThisSize++;
    }
    // If we have played enough times with this board size
    if (GAME->boardSize == GAME->timesPlayedOnThisSize) {
        GAME->boardSize++;
        GAME->timesPlayedOnThisSize = 1;
    }

    GAME->wrongTiles = 0;
    GAME->currentTileX = 0;
    GAME->currentTileY = 0;
    initializeBoard(GAME->boardSize, GAME->numOfTiles);
    // Let the pattern be shown to the player
    playerTurn = false;
}

void initializeBoard(unsigned boardSize, unsigned numOfTiles) {
    GAME->boardSize = boardSize;
    GAME->numOfTiles = numOfTiles;

    // Allocate memory
    GAME->board = (int**)malloc(boardSize*sizeof(int*));
    GAME->selectedTiles = (int**)malloc(boardSize*sizeof(int*));
    int row = 0;
    for (; row < boardSize; row++) {
        // Use calloc initialize them to 0
        GAME->board[row] = (int *)calloc(boardSize, sizeof(int));
        GAME->selectedTiles[row] = (int *)calloc(boardSize, sizeof(int));

    }

    for (row = 0; row < boardSize; row++) {
        for (int col = 0; col < boardSize; col++) {
            GAME->selectedTiles[row][col] = 0;
        }
    }
    
    // Set the random patterns
    int i = 0;
    for (; i < numOfTiles; i++) {
        int row = rand() % boardSize;
        int col = rand() % boardSize;
        while (GAME->board[row][col] == 1) {
            row = rand() % boardSize;
            col = rand() % boardSize;
        }
        GAME->board[row][col] = 1;
    }
}

void freeBoard(void) {
    int row = 0;
    for (; row < GAME->boardSize; row++) {
        free(GAME->board[row]);
    }
    free(GAME->board);
}


void selectTile(int row, int col) {
    // Ensure that the tile hasn't already been selected
    if (GAME->selectedTiles == 0) {
        if (GAME->board[row][col] == 1) {
            GAME->selectedTiles[row][col] = 1;
            GAME->tilesFound++;
        }
        else {
            GAME->selectedTiles[row][col] = 2;
            GAME->wrongTiles++;
        }
    }
    return;
}


// Checks if the level was passed
ClickResult checkBoard(void) {
    unsigned correctTiles = 0;
    unsigned row = 0;
    for (; row < GAME->boardSize; row++) {
        unsigned col = 0;
        for (; col < GAME->boardSize; col++) {
            if (GAME->selectedTiles[row][col]) {
                if (GAME->board[row][col] == 1) {
                    correctTiles++;
                }
            }
        }
    }
    if (correctTiles == GAME->numOfTiles){
        return PASS;
    }
    return STAY;
}
