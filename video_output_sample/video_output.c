#include "address_map_nios_v.h"

/* function prototypes */
void video_text(int, int, char *);
void video_box(int, int, int, int, short);
int resample_rgb(int, int);
int get_data_bits(int);
#define STANDARD_X 320
#define STANDARD_Y 240
#define INTEL_BLUE 0x0071C5

/* global variables */
int screen_x;
int screen_y;
int res_offset;
int col_offset;

int main(void) {
    volatile int * video_resolution = (int *)(PIXEL_BUF_CTRL_BASE + 0x8);
    screen_x = *video_resolution & 0xFFFF;
    screen_y = (*video_resolution >> 16) & 0xFFFF;
    // The following two lines are supported in hardware, but not in CPUlator
    volatile int * rgb_status = (int *)(RGB_RESAMPLER_BASE);
    int db = get_data_bits(*rgb_status & 0x3F);
    // int db = 16; // replace above two lines with this one for CPUlator
    /* check if resolution is smaller than the standard 320 x 240 */
    res_offset = (screen_x == 160) ? 1 : 0;
    /* check if number of data bits is less than the standard 16-bits */
    col_offset = (db == 8) ? 1 : 0;
    /* create a message to be displayed on the video and LCD displays */
    char text_top_row[40] = "Intel FPGA\0";
    char text_bottom_row[40] = "Computer Systems\0";
    /* update color */
    short background_color = resample_rgb(db, INTEL_BLUE);
    video_text(35, 29, text_top_row);
    video_text(32, 30, text_bottom_row);
    video_picture(0, 0, 320, 240, (short *)FPGA_PIXEL_BUF_BASE);
    video_box(0, 0, STANDARD_X, STANDARD_Y, 0); // clear the screen
    video_box(31 * 4, 28 * 4, 49 * 4 - 1, 32 * 4 - 1, background_color);
}

/*******************************************************************************
* Subroutine to send a string of text to the video monitor
******************************************************************************/
void video_text(int x, int y, char * text_ptr) {
    int offset;
    volatile char * character_buffer =(char *)FPGA_CHAR_BASE; // video character buffer
    /* assume that the text string fits on one line */
    offset = (y << 7) + x;
    while (*(text_ptr)) {
        *(character_buffer + offset) =
        *(text_ptr); // write to the character buffer
        ++text_ptr;
        ++offset;
    }
}

/*******************************************************************************
* Draw a filled rectangle on the video monitor
* Takes in points assuming 320x240 resolution and adjusts based on differences
* in resolution and color bits.
******************************************************************************/
void video_box(int x1, int y1, int x2, int y2, short pixel_color) {
    int pixel_buf_ptr = *(int *)PIXEL_BUF_CTRL_BASE;
    int pixel_ptr, row, col;
    int x_factor = 0x1 << (res_offset + col_offset);
    int y_factor = 0x1 << (res_offset);
    x1 = x1 / x_factor;
    x2 = x2 / x_factor;
    y1 = y1 / y_factor;
    y2 = y2 / y_factor;
    /* assume that the box coordinates are valid */
    for (row = y1; row <= y2; row++)
        for (col = x1; col <= x2; ++col) {
        pixel_ptr = pixel_buf_ptr +
        (row << (10 - res_offset - col_offset)) + (col << 1);
        *(short *)pixel_ptr = pixel_color; // set pixel color
    }
}
    /********************************************************************************
    * Resamples 24-bit color to 16-bit or 8-bit color
    *******************************************************************************/
int resample_rgb(int num_bits, int color) {
    if (num_bits == 8) {
        color = (((color >> 16) & 0x000000E0) | ((color >> 11) & 0x0000001C) |
        ((color >> 6) & 0x00000003));color = (color << 8) | color;
    } else if (num_bits == 16) {
        color = (((color >> 8) & 0x0000F800) | ((color >> 5) & 0x000007E0) |
        ((color >> 3) & 0x0000001F));
    }
    return color;
}
/********************************************************************************
* Finds the number of data bits from the mode
*******************************************************************************/
int get_data_bits(int mode) {
    switch (mode) {
        case 0x0:
        return 1;
        case 0x7:
        return 8;
        case 0x11:
        return 8;
        case 0x12:
        return 9;
        case 0x14:
        return 16;
        case 0x17:
        return 24;
        case 0x19:
        return 30;
        case 0x31:
        return 8;
        case 0x32:
        return 12;
        case 0x33:
        return 16;
        case 0x37:
        return 32;
        case 0x39:
        return 40;
    }
    return -1; // error
}

void video_picture(int x, int y, int width, int height, short * picture) {
    int pixel_buf_ptr = *(int *)PIXEL_BUF_CTRL_BASE;
    int pixel_ptr, row, col;
    int x_factor = 0x1 << (res_offset + col_offset);
    int y_factor = 0x1 << (res_offset);
    x = x / x_factor;
    y = y / y_factor;
    width = width / x_factor;
    height = height / y_factor;
    for (row = y; row < y + height; row++)
        for (col = x; col < x + width; col++) {
        pixel_ptr = pixel_buf_ptr +
        (row << (10 - res_offset - col_offset)) + (col << 1);
        *(short *)pixel_ptr = *picture; // set pixel color
        picture++;
    }
}