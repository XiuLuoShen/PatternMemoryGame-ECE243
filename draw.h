// Header file for drawing the game
#ifndef DRAW_H_   /* Include guard */
#define DRAW_H_

extern volatile int pixel_buffer_start; // global variable for pixel buffer
// Colors!

// Draw board
void drawBoard();

// Draw borders

void draw_line(int x0, int y0, int x1, int y1, short int color);
int abs(int value);
void swap(int *x, int *y);

// Draw a tile with top left corner at x, y with width size
void drawTile(int x, int y, int size, short int color);

void plot_pixel(int x, int y, short int line_color);
void clear_screen(void);


#endif // DRAW_H_
