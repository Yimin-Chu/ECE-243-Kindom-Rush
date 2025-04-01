#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// Include all necessary header files
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"
#include "monster_moving.h"

// Define the coordinate structure for tower placement
typedef struct {
    int x;
    int y;
} Coordinate;

// Predefined tower positions
Coordinate block_positions[10] = {
    {7, 39},    // Block 1
    {7, 88},    // Block 2
    {7, 137},   // Block 3
    {80, 60},   // Block 4
    {80, 109},  // Block 5
    {80, 158},  // Block 6
    {155, 72},  // Block 7
    {155, 137}, // Block 8
    {235, 60},  // Block 9
    {235, 109}  // Block 10
};


// Assume pixel_buffer_start is defined elsewhere (e.g., in plot_image.c)
extern int pixel_buffer_start;
bool released = false;

// Monster initial position and movement state
int monster_x = 49;   // Starting X (left-bottom of road)
int monster_y = 180;  // Starting Y
int dx = 0, dy = 0;   // Movement direction (will be set in update_monster_position)
bool monster_finished = false;


int main(void) {
    // 1) Initialize the PS/2 keyboard
    initPS2Keyboard();

    // 2) Get pointer to pixel controller memory
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    // Initially use single-buffering: point to Buffer1
    // *pixel_ctrl_ptr = (int)&Buffer1[0];
    pixel_buffer_start = *pixel_ctrl_ptr;
    
    // 3) Draw the Intro screen
    plot_image_intro(0, 0);

    // 4) Wait for the user to press Enter (scan code 0x5A)
    char code;
    while (1) {
        // Call readPS2ScanCode() to update the global ps2_keyboard_code
        code = readPS2ScanCode();
        if (code == 0x5A) {
            // Once Enter is pressed, switch to game screen (background with brown road)
            //plot_image_game(0, 0);
            break;
        }
    }

    // 5) Initialize double buffering:
    // Set front buffer pointer to Buffer1 and wait for vsync
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1[0];
    wait_for_vsync();  // Wait for vertical sync to complete the swap
    pixel_buffer_start = *pixel_ctrl_ptr;  // Now front buffer is Buffer1

    // Draw static elements (background, towers) on the front buffer
    plot_image_game(0, 0);



    // Set back buffer pointer to Buffer2 and draw identical static elements
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2[0];
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    plot_image_game(0, 0);
    // Optionally, add other static elements (e.g., plot_image_tower2)

    // 6) Enter main loop: process monster movement and tower placement concurrently
    while (1) {
        // Set back buffer as the current drawing target
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        // --- Monster Movement ---
        if (!monster_finished) {
            // Erase the previous monster image from the back buffer
            clear_drawn_pixels();
            // Update monster position (this function should change dx/dy as needed based on path)
            update_monster_position(&monster_x, &monster_y, &dx, &dy);
            // Check if monster has reached the finish line (adjust condition based on your road)
            if (monster_x > 280) {  
                monster_finished = true;
            } else {
                // Draw the monster at its new position
                
                plot_image_monster(monster_x, monster_y);
            }
        }

        // --- Tower Placement ---
        // Read the PS/2 scan code (this call updates the global variable ps2_keyboard_code)
        // In main loop...
        char code = readPS2ScanCode();
        if (code != 0) {
            // Process the key
            if (code == 0xF0) {
                released = true;
            } else {
                if (!released) {
                    int block_idx = get_block_index_from_scan_code(code);
                    if (block_idx >= 0 && block_idx < 10) {
                        Coordinate pos = block_positions[block_idx];
                        plot_image_tower1(pos.x, pos.y);
                    }
                }
                released = false;
            }
        }


        // --- Swap Buffers ---
        wait_for_vsync();
        // After vsync, the back buffer becomes the front buffer.
        // Continue drawing on the new back buffer in the next iteration.
    }

    return 0;
}
