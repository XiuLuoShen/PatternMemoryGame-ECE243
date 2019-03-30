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
    volatile int * PS2Ptr = (int *) 0xFF200100;

    // Reset the mouse
    *PS2Ptr = 0xFF;
    unsigned char temp = (*PS2Ptr);
    while (temp != 0xAA) {
        temp = (*PS2Ptr);
    }

    // Enable Interrupts
    *(PS2Ptr + 1) = 0x1;

    // Enable data sending
    *PS2Ptr = 0xF4;
}

void mouseISR(void) {
    // Check that there are only 3 packets
    volatile int * PS2Ptr = (int *) 0xFF200100;

    volatile int packet = *PS2Ptr;

    unsigned char packet0 = packet & 0xFF;
    while (packet >> 16 != 0) {
        int packetNumber = packet >> 16 % 3;
        unsigned char data = packet & 0xFF;
        if (packetNumber == 0) {
            packet0 = data & 0xFF;
            // If left mouse button was clicked
            if (packet0 & 0x01) {
                mouseClicked = 1;
            }
        }
        else if (packetNumber == 1) {
            // packet0 contains the sign of the x movement in bit 4, so we shift it left by 4 and then shift it right by 8
            if ((packet0 >> 4 & 0b1) == 1) {
                // Get magnitude and then subtract
                unsigned xChange = data^0xFF;
                xChange += 1;
                xPos -= xChange;
            }
            else {
                xPos += data;
            }
        }
        else if (packetNumber == 2) {
            // Packet 0 contains sign of the y movement in bit 5 so we shift it left by 3
            if ((packet0 >> 5 & 0b1) == 1) {
                // Get magnitude and then subtract
                unsigned yChange = data^0xFF;
                yChange += 1;
                yPos -= yChange;
            }
            else {
                yPos += data;
            }
        }
    }

    if (xPos < 0) {
        xPos = 0;
    }
    else if (xPos > 320) {
        xPos = 320;
    }

    if (yPos < 0) {
        yPos = 0;
    }
    else if (yPos > 240) {
        yPos = 240;
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
    else if (interrupt_ID == PS2_BASE) {
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
