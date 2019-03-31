// Implementation of interrupts
#include <stdlib.h>

#include "interrupts.h"
#include "game.h"
#include "address_map_arm.h"

void configA9Timer(void) {
    volatile int * timerPtr = (int *) 0xFFFEC600;

    // Set the timer to count for 2s
    *(timerPtr) = 400000000;
    // Make the timer reload the value
    *(timerPtr+2) |= 0b10;
}


void configMouse(void) {
    volatile int * PS2_ptr = (int *)PS2_BASE; // PS/2 port address

    *(PS2_ptr) = 0xFF; /* reset */
	*PS2_ptr = 0xE8;	// Set Resolution
	*PS2_ptr = 0x00;	// Resolution
	*PS2_ptr = 0xEA;

    *(PS2_ptr + 1) = 0x1; /* write to the PS/2 Control register to enable interrupts */

}

void mouseISR(void) {
    // Check that there are only 3 packets
    volatile int * PS2_ptr = (int *) 0xFF200100;
    int PS2_data, RAVAIL;
	PS2_data = *(PS2_ptr); // read the Data register in the PS/2 port
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;			// extract the RAVAIL field

	// byteNumber = (RAVAIL-1) % 3;
	// printf("Byte number %d\n", RAVAIL);
	if (*(PS2_ptr) == 0xFA){
		byteNumber = 0;
		return;
	}

	if (RAVAIL){
        if (byteNumber == 0) {
           byte1 = *(PS2_ptr) & 0xFF;
           if (byte1 & 0x01) {
               mouseClicked = 1;
           }
       }

       // If X movement
        if (byteNumber == 1) {
             byte2 = *(PS2_ptr) & 0xFF;
            // If no overflow
             if (!(byte1 & 0x40)) {
                 if (byte1 & 0x10) {
                    // If negative, then subtract the magnitude of x movement
					xPos -= (byte2 ^ 0xFF) + 1;
					// if (byte2^0xFF > 4)
						// xPos -= 2;
                }
                else {
					// if (byte2 > 4)
						// xPos += 2;
					xPos += byte2;
                }
             }
         }

		// If Y movement
		if (byteNumber == 2) {
			byte3 = *(PS2_ptr) & 0xFF;
			// If no overflow
			 if (!(byte1 & 0x80)) {
				if (byte1 & 0x20) {
					// If negative, then subtract the magnitude of x movement
					// Mouse movement up is positive, but on screen should be negative
					// yPos += (unsigned)((byte2 ^ 0xFF) + 1);
					// if ((byte3^0xFF) + 1 > 4)
						// yPos += 2;
					yPos += (unsigned)(byte3 ^ 0xFF) + 1;
				}
				else {
					// yPos -= (unsigned)byte2;
					// if (byte3 > 4)
						// yPos -= 2;
					yPos -= (unsigned) byte3;
				}
			 }
		}

		if (xPos < 0) {
			xPos = 0;
		}
		else if (xPos >= 320) {
			xPos = 319;
		}

		if (yPos < 0) {
			yPos = 0;
		}
		else if (yPos >= 240) {
			yPos = 239;
		}

		byteNumber++;
		if (byteNumber == 3) {
			byteNumber = 0;
		}
    }

}

/*************** The code below was taken from the Using_GIC.pdf handout **********************/

// Define the IRQ exception handler
void __attribute__((interrupt)) __cs3_isr_irq(void) {
    // Read the ICCIAR from the CPU Interface in the GIC
    int interrupt_ID = *((int *)0xFFFEC10C);
    if (interrupt_ID == 73) { // check if interrupt is from the KEYs
        pushbutton_ISR();
    }
    else if (interrupt_ID == 79) {
        mouseISR();
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
