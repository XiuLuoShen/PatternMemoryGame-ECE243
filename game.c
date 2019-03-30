#include <stdlib.h>

#include "game.h"

void restartGame(void) {
    GAME->level = 1;
    GAME->lives = 3;
}

void startLevel(unsigned level) {
    GAME->level = level;
    GAME->wrongTiles = 0;
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
