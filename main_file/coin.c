#include "address_map_nios_v.h"
#include "coin.h"
#include <stdbool.h>

// Example 7-segment decode table for active-low hardware.
// Each entry is the pattern to display the digit in bits 6..0 (bit 7 is the decimal point, here set to 1).
static const unsigned char seven_seg_decode_table[10] = {
    0x3F, // 0 -> 0b00111111 (active-low: segments a,b,c,d,e,f on)
    0x06, // 1 -> 0b00000110
    0x5B, // 2 -> 0b01011011
    0x4F, // 3 -> 0b01001111
    0x66, // 4 -> 0b01100110
    0x6D, // 5 -> 0b01101101
    0x7D, // 6 -> 0b01111101
    0x07, // 7 -> 0b00000111
    0x7F, // 8 -> 0b01111111
    0x6F  // 9 -> 0b01101111
};

int coin = 40; // Initial coin count
/*
 * Display a 3-digit decimal number on HEX2..HEX0, and blank out HEX3.
 * 'value' should be in range 0..999 for correct display.
 */
void display_3digits_on_HEX(int value) {
    volatile int *HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;  // 0xFF200020

    // Break 'value' into hundreds, tens, and ones
    int hundreds = (value / 100) % 10;
    int tens = (value / 10) % 10;
    int ones = value % 10;

    // Get the 7-seg code for each digit
    unsigned char seg_hundreds = seven_seg_decode_table[hundreds];
    unsigned char seg_tens     = seven_seg_decode_table[tens];
    unsigned char seg_ones     = seven_seg_decode_table[ones];

    // HEX3 will be blank (0xFF for active-low = all segments off)
    unsigned char seg_blank    = 0x3F;

    // Combine into a 32-bit word:
    //  bits 31..24 -> HEX3
    //  bits 23..16 -> HEX2
    //  bits 15..8  -> HEX1
    //  bits 7..0   -> HEX0
    int combined = (seg_blank << 24)
                 | (seg_hundreds << 16)
                 | (seg_tens << 8)
                 | (seg_ones);

    // Write to the parallel port controlling HEX3..HEX0
    *HEX3_HEX0_ptr = combined;
}

void add_coin(void) {
    coin += 20;
    display_3digits_on_HEX( coin );
}

bool spend_coin(void) {
    if (coin >= 40) {
        coin -= 40;
        display_3digits_on_HEX( coin );
        return true;
    } else {
        // Not enough coins to place a tower.
        return false;
    }
}

int get_coin(void) {
    return coin;
}

bool can_place_tower(void) {
    return (coin >= 40); // Check if there are enough coins to place a tower
}