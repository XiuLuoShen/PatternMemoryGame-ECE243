// Header file for interrupts / devices
#ifndef INTERRUPTS_H_   /* Include guard */
#define INTERRUPTS_H_

// Interrupt Configuration/Setup
void disable_A9_interrupts(void);
void set_A9_IRQ_stack(void);
void enable_A9_interrupts(void);
// GIC
void config_GIC(void);
void config_interrupt(int N, int CPU_target);

void pushbutton_ISR(void);

// VGA: Self-explanatory

// TIMER: Delays to show the pattern to the user
void configA9Timer(void);

// Keyboard
void keyboardISR(void);


// MOUSE: User Input
extern volatile int xPos;
extern volatile int yPos;
extern volatile int mouseClicked;
extern volatile unsigned char byte1, byte2, byte3;
extern volatile int byteNumber;

void configPS2(void);
void mouseISR(void);

#endif // INTERRUPTS_H_
