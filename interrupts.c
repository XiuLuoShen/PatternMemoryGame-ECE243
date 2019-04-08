// Implementation of interrupts and use of devices
#include <stdlib.h>
#include <stdbool.h>

#include "interrupts.h"
#include "game.h"
#include "keyboard_codes.h"
#include "address_map_arm.h"

// Configure timer
void configA9Timer(void) {
    volatile int * timerPtr = (int *) 0xFFFEC600;

    // Set the timer to count for 2s
    *(timerPtr) = 400000000;
    // Make the timer reload the value
    *(timerPtr+2) |= 0b10;
}

// Delay using the timer
void delayms(int ms) {
    // disable_A9_interrupts(); // disable interrupts in the A9 processor

    volatile int * timerPtr = (int *) 0xFFFEC600;

    // Load value
    *(timerPtr) = ms * 200000;
    // Current value
    // *(timerPtr+1) = ms * 2000000;

    // Set enable
    *(timerPtr + 2) = 0b1;
    // Wait for the timer to count down, poll for the interrupt status
    while (*(timerPtr+3) != 0b1) {

    }
    // Clear the interrupt flag
    *(timerPtr+3) = 0b1;

    // enable_A9_interrupts(); // enable interrupts in the A9 processor
}

// Configure PS2
void configPS2(void) {
    volatile int * PS2_ptr = (int *)PS2_BASE; // PS/2 port address

    *(PS2_ptr) = 0xFF; /* reset */
	// *PS2_ptr = 0xE8;	// Set Resolution
	// *PS2_ptr = 0x00;	// Resolution
	// *PS2_ptr = 0xEA;

    *(PS2_ptr + 1) = 0x1; /* write to the PS/2 Control register to enable interrupts */

}

extern volatile char keyByte1, keyByte2, keyByte3;

/**
 * Interrupt routine for the keyboard
 * Only key releases are used for input
 */
void keyboardISR(void) {
    volatile int * PS2_ptr = (int *) 0xFF200100;		// PS/2 port address
	int PS2_data, RAVAIL;

	PS2_data = *(PS2_ptr); // read the Data register in the PS/2 port
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;			// extract the RAVAIL field
	if (RAVAIL > 0)
	{
		/* always save the last three bytes received */
		keyByte1 = keyByte2;
		keyByte2 = keyByte3;
		keyByte3 = PS2_data & 0xFF;
	}

    if (keyByte1 == EXTENDED_KEYS) {
        if (keyByte2 == BREAK_CODE) {
            if (playerTurn) {
                if (keyByte3 == RIGHT_ARROW) {
                    GAME->currentTileX++;
                    if (GAME->currentTileX >= GAME->boardSize) {
                        GAME->currentTileX = 0;
                    }
                }
                else if (keyByte3 == LEFT_ARROW) {
                    GAME->currentTileX--;
                    if (GAME->currentTileX < 0) {
                        GAME->currentTileX = GAME->boardSize - 1;
                    }
                }
                else if (keyByte3 == UP_ARROW) {
                    GAME->currentTileY--;
                    if (GAME->currentTileY < 0) {
                       GAME->currentTileY = GAME->boardSize - 1;
                    }
                }
                else if (keyByte3 == DOWN_ARROW) {
                    GAME->currentTileY++;
                    if (GAME->currentTileY >= GAME->boardSize) {
                        GAME->currentTileY = 0;
                    }
                }
            }
        }
    }
    else if (keyByte2 == BREAK_CODE) {
        if (keyByte3 == ENTER_KEY) {
            // If it is at the very start of the game
            if (!started) {
                started = true;
                playerTurn = false;
                return;
            }

            if (!lost) {
                if (playerTurn) {
                    selectTile(GAME->currentTileY, GAME->currentTileX);
                }
            }
            else { // If game was lost, can restart by pressing enter
                started = true;
                freeBoard();
                restartGame();
            }
        }
    }
    return;
}


/* void keyboardISR(void) {
    volatile int * PS2_ptr = (int *) PS2_BASE;
    volatile int readRegister;
    volatile int RVALID;
    volatile char data;

    while (*(PS2_ptr+1) & 0x100) {
        do {
            readRegister = *PS2_ptr;
            RVALID = readRegister & 0x8000;
        } while (!RVALID);

        data = readRegister& 0xFF;
        if (data == EXTENDED_KEYS) {
            // Extended key press
            do {
                readRegister = *PS2_ptr;
                RVALID = readRegister & 0x8000;
            } while (!RVALID);
            data = readRegister & 0xFF;

            bool keyReleased = false;
            if (data == BREAK_CODE) {
                // Break data was sent
                keyReleased = true;
                do {
                    readRegister = *PS2_ptr;
                    RVALID = readRegister & 0x8000;
                } while (!RVALID);
                data = readRegister & 0xFF;
            }

            if (keyReleased && playerTurn) {
                if (data == RIGHT_ARROW) {
                    // Right arrow pressed
                    // Selected tile moves right
                    GAME->currentTileX++;
                    if (GAME->currentTileX >= GAME->boardSize) {
                        GAME->currentTileX = 0;
                    }
                }
                else if (data == LEFT_ARROW) {
                    // Left arrow pressed
                    // Selected tile moves left
                    GAME->currentTileX--;
                    if (GAME->currentTileX < 0) {
                        GAME->currentTileX = GAME->boardSize - 1;
                    }
                }
                else if (data == UP_ARROW) {
                    // Up arrow pressed
                    // Selected tile moves up
                    GAME->currentTileY--;
                    if (GAME->currentTileY < 0) {
                        GAME->currentTileY = GAME->boardSize - 1;
                    }
                }
                else if (data == DOWN_ARROW) {
                    // Down arrow pressed
                    // Selected tile moves down
                    GAME->currentTileY++;
                    if (GAME->currentTileY >= GAME->boardSize) {
                        GAME->currentTileY = 0;
                    }
                }
            }
        }
        else if (data != EXTENDED_KEYS && data == BREAK_CODE){
            // Non extended key press
            // Break data was sent
            do {
                readRegister = *PS2_ptr;
                RVALID = readRegister & 0x8000;
            } while (!RVALID);

            data = readRegister & 0xFF;

            if (data == ENTER_KEY) {
                // Enter key was pressed
                // If still playing

                // if (!lost && started) {
                if (!lost) {
                    selectTile(GAME->currentTileX, GAME->currentTileY);
                }
                else { // If game was lost, can restart by pressing enter
                    started = true;
                    freeBoard();
                    restartGame();
                }
            }
        }
    }
} */



/*************** The code below was taken from the Using_GIC.pdf handout **********************/

// Define the IRQ exception handler
void __attribute__((interrupt)) __cs3_isr_irq(void) {
    // Read the ICCIAR from the CPU Interface in the GIC
    int interrupt_ID = *((int *)0xFFFEC10C);
    if (interrupt_ID == 73) { // check if interrupt is from the KEYs
        pushbutton_ISR();
    }
    else if (interrupt_ID == 79) {
        keyboardISR();
    }
    else {
        // If unexpected, then clear the memory and stay here
        free(GAME);
        while (1);
    }
    // Write to the End of Interrupt Register (ICCEOIR)
    *((int *)0xFFFEC110) = interrupt_ID;
}

// Define the remaining exception handlers
void __attribute__((interrupt)) __cs3_reset(void) {
    while (1);
}
void __attribute__((interrupt)) __cs3_isr_undef(void) {
    while (1);
}
void __attribute__((interrupt)) __cs3_isr_swi(void) {
    while (1);
}
void __attribute__((interrupt)) __cs3_isr_pabort(void) {
    while (1);
}
void __attribute__((interrupt)) __cs3_isr_dabort(void) {
    while (1);
}
void __attribute__((interrupt)) __cs3_isr_fiq(void) {
    while (1);
}

/*
* Turn off interrupts in the ARM processor
*/
void disable_A9_interrupts(void) {
    int status = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}

/*
* Initialize the banked stack pointer register for IRQ mode
*/
void set_A9_IRQ_stack(void) {
    int stack, mode;
    stack = 0xFFFFFFFF - 7; // top of A9 onchip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010;
    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r"(stack));
    /* go back to SVC mode before executing subroutine return! */
    mode = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
}

/*
* Turn on interrupts in the ARM processor
*/
void enable_A9_interrupts(void) {
    int status = 0b01010011;
    asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}

/*
* Configure the Generic Interrupt Controller (GIC)
*/
void config_GIC(void) {
    config_interrupt (73, 1); // configure the FPGA KEYs interrupt (73)
	config_interrupt (79, 1); // configure the FPGA PS2 interrupt (79)
    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
    // priorities
    *((int *) 0xFFFEC104) = 0xFFFF;
    // Set CPU Interface Control Register (ICCICR). Enable signaling of
    // interrupts
    *((int *) 0xFFFEC100) = 1;
    // Configure the Distributor Control Register (ICDDCR) to send pending
    // interrupts to CPUs
    *((int *) 0xFFFED000) = 1;
}

/*
* Configure Set Enable Registers (ICDISERn) and Interrupt Processor Target
* Registers (ICDIPTRn). The default (reset) values are used for other registers
* in the GIC.
*/
void config_interrupt(int N, int CPU_target) {
    int reg_offset, index, value, address;
    /* Configure the Interrupt Set-Enable Registers (ICDISERn).
    * reg_offset = (integer_div(N / 32) * 4
    * value = 1 << (N mod 32) */
    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    /* Now that we know the register address and value, set the appropriate bit */
    *(int *)address |= value;
    /* Configure the Interrupt Processor Targets Register (ICDIPTRn)
    * reg_offset = integer_div(N / 4) * 4
    * index = N mod 4 */
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;
    /* Now that we know the register address and value, write to (only) the
    * appropriate byte */
    *(char *)address = (char)CPU_target;
}


void pushbutton_ISR(void) {
    /* KEY base address */
    volatile int * KEY_ptr = (int *) 0xFF200050;
    /* HEX display base address */
    volatile int * HEX3_HEX0_ptr = (int *) 0xFF200020;
    int press, HEX_bits;
    press = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press; // Clear the interrupt
    if (press & 0x1) // KEY0
    HEX_bits = 0b00111111;
    else if (press & 0x2) // KEY1
    HEX_bits = 0b00000110;
    else if (press & 0x4) // KEY2
    HEX_bits = 0b01011011;
    else // press & 0x8, which is KEY3
    HEX_bits = 0b01001111;
    *HEX3_HEX0_ptr = HEX_bits;
    return;
}
