#include <stdlib.h>

#include "address_map_arm.h"
#include "game.h"
#include "interrupts.h"
#include "draw.h"

void initializeBuffers(void);
void wait_for_vsync(void);
void HEX_PS2(char, char, char);


// Global variables
volatile int pixel_buffer_start; // global variable from draw.h
Game* GAME; // from game.h
bool lost = false;
bool started = false;   // For when the program is loaded the first time
char keyByte1, keyByte2, keyByte3;  // The bytes from the keyboard

int main(void) {

    keyByte1 = 0;
    keyByte2 = 0;
    keyByte3 = 0;

    // Allocate the memory for the game struct and initialize
    GAME = (Game*) malloc(sizeof(Game));
    restartGame();  // Allocation of memory for the game

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

        // Draw text to start the game
        if (started) {
            GAME->level = 0;
        }
        else if (lost) {
            // Draw the lost/restart text
        }
        draw_text(GAME->level, GAME->lives);

        // Draw the game board
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

        HEX_PS2(keyByte1, keyByte2, keyByte3);
        // If the game has not been started yet
        if (!started) {
            continue;
        }
        // If the game has not been lost
        if (!lost) {
            // Check if pattern is being shown to player
            if (!playerTurn) {
                // Let the pattern be shown for 2s
                delayms(2000);
                playerTurn = true;
            }
            // Check if  level was passed
            if (GAME->tilesFound >= GAME->numOfTiles) {
                delayms(300);
                newLevel(GAME->level + 1);
                playerTurn = false;
            }
            // Check if level was failed
            else if (GAME->wrongTiles >= 3) {
                delayms(300);
                GAME->lives--;
                // Check if the game has been lost
                if (GAME->lives == 0) {
                    lost = true;
                    playerTurn = false;
                }
                // If not lost then try the level again
                else {
                    newLevel(GAME->level);
                }
                playerTurn = false;
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




void HEX_PS2(char b1, char b2, char b3) {
    volatile int * HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
    volatile int * HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;

    /* SEVEN_SEGMENT_DECODE_TABLE gives the on/off settings for all segments in
     * a single 7-seg display in the DE2 Media Computer, for the hex digits 0 -
     * F */
    unsigned char seven_seg_decode_table[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
        0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int  shift_buffer, nibble;
    unsigned char code;
    int           i;

    shift_buffer = (b1 << 16) | (b2 << 8) | b3;
    for (i = 0; i < 6; ++i) {
        nibble = shift_buffer & 0x0000000F; // character is in rightmost nibble
        code   = seven_seg_decode_table[nibble];
        hex_segs[i]  = code;
        shift_buffer = shift_buffer >> 4;
    }
    /* drive the hex displays */
    *(HEX3_HEX0_ptr) = *(int *)(hex_segs);
    *(HEX5_HEX4_ptr) = *(int *)(hex_segs + 4);
}
