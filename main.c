#include <stdlib.h>

#include "address_map_arm.h"
#include "game.h"
#include "interrupts.h"
#include "draw.h"

void initializeBuffers(void);
void wait_for_vsync(void);

// Global variables
volatile int pixel_buffer_start; // global variable from draw.h
Game* GAME; // from game.h
bool lost = false;

int main(void) {
    volatile int* ledr = (int *) 0xFF200000;

    // Allocate the memory for the game struct and initialize
    GAME = (Game*) malloc(sizeof(Game));
    restartGame();

    disable_A9_interrupts(); // disable interrupts in the A9 processor
    set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
    config_GIC(); // configure the general interrupt controller
    configPS2();    // Configure the PS2 port

    // Configure VGA stuff
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    initializeBuffers();

    enable_A9_interrupts(); // enable interrupts in the A9 processor

    // Main game loop
    while (1) {
        // Clear the screen
        clear_screen();

        // If statement to draw the pattern or the board
        drawBoard(playerTurn);

        if (lost) {
            // Draw the lost/restart text
        }

        // Draw the game board
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

        // If the game has not been lost
        if (!lost) {
            // Check if pattern is being shown to player
            if (!playerTurn) {
                // Let the pattern be shown for 2s
                delayms(2000);
                playerTurn = true;
            }
            // Check if  level was passed
            if (GAME->tilesFound == GAME->numOfTiles) {
                delayms(300);
                newLevel(GAME->level + 1);
            }
            // Check if level was failed
            else if (GAME->wrongTiles >= 3) {
                delayms(300);
                GAME->lives--;
                // Check if the game has been lost
                if (GAME->lives == 0) {
                    lost = true;
                }
                // If not lost then try the level again
                else {
                    newLevel(GAME->level);
                }
            }
        }
    }

   return 0;
}

void initializeBuffers(void) {
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
}

void wait_for_vsync(void) {
    // pixel controller
    volatile int * pixel_ctrl_ptr = (int *) 0xFF203020;
    register int status;
    *pixel_ctrl_ptr = 1; // start syncronization

    status = *(pixel_ctrl_ptr+3);   // +3 because its an int *
    while ((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr+3);   // +3 because its an int *
    }
}
