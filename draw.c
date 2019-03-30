//
#include "draw.h"
#include "game.h"


// Draws the board and borders
void drawBoard(void) {
    // Draw the top border?
    for (int row = 0; row < GAME->boardSize; row++) {
        for (int col = 0; col < GAME->boardSize; col++) {

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

    for (int x = x0 ; x <= x1; x++) {
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

void drawTile(int x, int y, int size, short int color) {
    for (int row = x; row < x + size; row++) {
        for (int col = y; col < y + size; col++) {
            plot_pixel(x, y, color);
        }
    }
}

void clear_screen(void){
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, 0x0);
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
