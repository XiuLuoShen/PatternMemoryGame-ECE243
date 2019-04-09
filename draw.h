// Header file for drawing the game
#ifndef DRAW_H_   /* Include guard */
#define DRAW_H_

#define correctColor 0xFFFF
#define unselectedColor 0x1C3F
#define wrongColor 0x088B
#define backgroundColor 0x459F

extern volatile int pixel_buffer_start; // global variable for pixel buffer
// Colors!

// Draw board
void drawBoard(int playersTurn);

// Draw borders
void draw_board_background(void);

void draw_line(int x0, int y0, int x1, int y1, short int color);
int abs(int value);
void swap(int *x, int *y);

// Draw a tile with top left corner at x, y with width size
void drawTile(int x, int y, int size, short int color);

void plot_pixel(int x, int y, short int line_color);
void clear_screen(void);

void draw_text(int level, int lives);

void draw_game_over();

void draw_game_start();
#endif // DRAW_H_
