//
#include "draw.h"

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
