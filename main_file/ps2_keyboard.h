#ifndef __PS2_KEYBOARD_H__
#define __PS2_KEYBOARD_H__

// Define the base address for the PS/2 controller
#define PS2_BASE 0xFF203000  

// Global variable to hold the last read scan code
extern volatile char ps2_keyboard_code;

// Function prototypes
void initPS2Keyboard(void);
char readPS2ScanCode(void);
int get_block_index_from_scan_code(char code);

#endif  /* __PS2_KEYBOARD_H__ */
