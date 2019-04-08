// This file contains implementation for drawing
#include "draw.h"
#include "game.h"

// TODO: Highlight selected square

// Draws the board and borders
void drawBoard(int playersTurn) {
    // Draw the top border?
    draw_board_background();


    int size = GAME->boardSize;;
    int borderWidth = 3;
    int squareSize = (240 - borderWidth*(GAME->boardSize + 1)) / GAME->boardSize;

    int row = 0;
    int yPos = borderWidth;
    for (; row < size; row++) {
        int xPos = 39 + borderWidth;
        int col = 0;
        for (; col < size; col++) {
            //color choices: the tile is inactive
            // the tile is selected (showing the pattern or it has been correctly selected)
            // the tile is incorrectly selected
            short int color;

            // the player is selecting the tiles
            if(playersTurn == 1 ){
              if(GAME->selectedTiles[row][col] == 0){
                color = unselectedColor;
              }
              else if (GAME->selectedTiles[row][col] == 1){
                color = correctColor;
              }
              else if(GAME->selectedTiles[row][col] == 2){
                color = wrongColor;
              }
            }
            else{ // the pattern is being shown
              if(GAME->board[row][col]){
                color = correctColor;
              }
              else{
                color = unselectedColor;
              }
            }

            // Highlight the selected square
            if (row == GAME->currentTileX && col == GAME->currentTileY) {
                draw_line(xPos-1, yPos-1, xPos + squareSize + 1, yPos-1, 0xFF66);
                draw_line(xPos-1, yPos + squareSize + 1, xPos + squareSize + 1, yPos + squareSize + 1, 0xFF66);
                draw_line(xPos-1, yPos-1, xPos - 1, yPos + squareSize + 1, 0xFF66);
                draw_line(xPos + squareSize + 1, yPos-1, xPos + squareSize + 1, yPos + squareSize + 1, 0xFF66);
            }

            drawTile(xPos, yPos, squareSize, color);
            xPos = xPos + borderWidth + squareSize;
        }
        yPos = yPos + borderWidth + squareSize;
    }
}

//draws a 240x240 blue background in the center of the screen
void draw_board_background(void){
  int x = 39;
  for (; x < 280; x++) {
      int y = 0;
      for (; y < 240; y++) {
          plot_pixel(x, y,backgroundColor);
      }
    }
}

//draws the tiles of the board, and their respective colors corresponding to status
//starting from top left corner
void drawTile(int x, int y, int size, short int color){
    int row = x;
    for(; row < x +size; row++){
        int col = y;
        for(; col < y +size; col ++){
          plot_pixel(row, col, color);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int color) {
    int is_steep = abs(y1-y0) > abs(x1-x0);
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int deltax = x1 - x0;
    int deltay = abs(y1-y0);
    int error = - (deltax / 2);
    int y = y0;
    int y_step;
    if (y0 < y1)    y_step = 1;
    else        y_step = -1;
    int x = x0;
    for (; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, color);
        }
        else {
            plot_pixel(x, y, color);
        }
        error += deltay;
        if (error >= 0){
            y += y_step;
            error -=  deltax;
        }
    }
}

void clear_screen(void){
    int x = 0;
    for (; x < 320; x++) {
        int y = 0;
        for (; y < 240; y++) {
            plot_pixel(x, y, 0x00);
        }
    }
}


void plot_pixel(int x, int y, short int line_color)
{
    // Error checking
    if (x < 0 || x >= 320 || y < 0 || y >= 240){
        return;
    }
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}


void swap(int *x, int *y) {
    int temp = *x;
    *x = *y;
    *y = temp;
}

int abs(int value) {
    if (value < 0) {
        value *= -1;
    }
    return value;
}

// void find_selected_tile(int x, int y){
//     int size = GAME->boardSize;
//     int borderWidth = 3;
//     int squareSize = (240 - borderWidth*(GAME->boardSize + 1)) / GAME->boardSize;
//
//     int selectedTileX = -1;
//     int selectedTileY = -1;
//     int i = 0;
//     for(; i < size; i++){
//         if(  ((i+1)*borderWidth + i*squareSize) < x && x < (i+1)*(borderWidth + squareSize)){
//             selectedTileX = i;
//         }
//         if(  ((i+1)*borderWidth + i*squareSize) < y && y < (i+1)*(borderWidth + squareSize)){
//             selectedTileY = i;
//         }
//     }
//
//     //did not click one of the tiles
//     if(selectedTileX == -1 || selectedTileY == -1){
//
//     }
//     }

void draw_text(int level, int lives){
  char* levels_text = "Level \0";
  char* lives_text = "Lives \0";

  int y = 29;
  int x = 2;
//Writes levels
  int
  char* text_ptr = levels_text;
  offset = (y << 7) + x;
  while (*(text_ptr)) {
    *(character_buffer + offset) = *(text_ptr); // write to the character buffer
    ++text_ptr;
    ++offset;
  }
  offset = (y << 7) + x + 7;
  *(character_buffer + offset) = '0' + level;

  y = 30;
  x = 2;
  //Writes lives
  char* text_ptr = lives_text;
  offset = (y << 7) + x;
  while (*(text_ptr)) {
    *(character_buffer + offset) = *(text_ptr); // write to the character buffer
    ++text_ptr;
    ++offset;
  }
  offset = (y << 7) + x + 7;
  *(character_buffer + offset) = '0' + lives;

}
