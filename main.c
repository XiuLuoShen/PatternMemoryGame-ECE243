#include <stdlib.h>

#include "address_map_arm.h"
#include "interrupts.h"
#include "draw.h"


void wait_for_vsync(void);

// Global variables
volatile int pixel_buffer_start; // global variable

int main(void) {
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


    return 0;
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
