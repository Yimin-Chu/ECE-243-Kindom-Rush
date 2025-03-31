#include "ps2_keyboard.h"
#include "address_map_nios_v.h"

volatile char ps2_keyboard_code = 0;  // 定义全局变量

void initPS2Keyboard(void) {
    volatile int *PS2_ptr = (int *)PS2_BASE;
    *PS2_ptr = 0xFF;  // Send reset command
}

char readPS2ScanCode(void) {
    volatile int *PS2_ptr = (int *)PS2_BASE;
    int PS2_data, RVALID;
    char scanCode;
    
    // Wait until valid data is available
    do {
        PS2_data = *PS2_ptr;
        RVALID = PS2_data & 0x8000;
    } while (!RVALID);
    
    // Extract the scan code
    scanCode = PS2_data & 0xFF;
    ps2_keyboard_code = scanCode;
    
    // If the self-test passed (0xAA), send the enable scanning command (0xF4)
    if (scanCode == 0xAA) {
        *PS2_ptr = 0xF4;
    }
    return scanCode;
}

int get_block_index_from_scan_code(char code) {
    switch (code) {
        case 0x16: return 0; // 1
        case 0x1E: return 1; // 2
        case 0x26: return 2; // 3
        case 0x25: return 3; // 4
        case 0x2E: return 4; // 5
        case 0x36: return 5; // 6
        case 0x3D: return 6; // 7
        case 0x3E: return 7; // 8
        case 0x46: return 8; // 9
        case 0x45: return 9; // 0
        case 0x4D: return 10; // - (optional)
        default: return -1;
    }
}
