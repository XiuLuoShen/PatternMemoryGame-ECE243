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
volatile int xPos;
volatile int yPos;
volatile int mouseClicked;
volatile int byteNumber;
volatile unsigned char byte1, byte2, byte3;
bool playerTurn = false;
bool lost = false;

int main(void) {
    volatile int* ledr = (int *) 0xFF200000;
    xPos = 160;
    yPos = 120;
    byteNumber = 0;
    byte1 = 0;
    byte2 = 0;
    byte3 = 0;

    disable_A9_interrupts(); // disable interrupts in the A9 processor
    set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
    config_GIC(); // configure the general interrupt controller
    configMouse();

    // Configure VGA stuff
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    initializeBuffers();

    // Allocate the memory for the game struct
    GAME = (Game*) malloc(sizeof(Game));
    GAME->lives = 3;
    GAME->level = 1;
    initializeBoard(3, 3);

    enable_A9_interrupts(); // enable interrupts in the A9 processor

	volatile int * PS2_ptr = (int *)PS2_BASE; // PS/2 port address
	// *(PS2_ptr) = 0xF4;	// Enable data sending

    // Main game loop
    while (1) {
		// byteNumber = -1;
        clear_screen();

        // If statement to draw the pattern or the board

        drawBoard(playerTurn);
        // Draw the pixel the mouse is at


        // Draw the game board
        // Draw the mouse
        // Poll for mouse info?
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }

    // // //TESTING DRAWING
    // initializeBoard(3,0);
    // GAME->selectedTiles[0][0] = 1;
    // GAME->selectedTiles[0][1] = 0;
    // GAME->selectedTiles[0][2] = 2;
    // drawBoard(1);
    //
    // wait_for_vsync();
    // Free memory used for the game struct, memory for board freed already
    free(GAME);


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
