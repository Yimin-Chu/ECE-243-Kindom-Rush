// #include "address_map_nios_v.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "plot_image.h"

volatile int pixel_buffer_start;

int main() {
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
}

//game
void plot_image_game(int x, int y) {
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 320; j++) {
            plot_pixel(x + j, y + i, game[i * 320 + j]);
        }
    }
}

void erase_image_game(int x, int y) {
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 320; j++) {
            plot_pixel(x + j, y + i, 0);
        }
    }
}

//intro
void plot_image_intro(int x, int y) {
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 320; j++) {
            plot_pixel(x + j, y + i, intro[i * 320 + j]);
        }
    }
}

void erase_image_intro(int x, int y) {
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 320; j++) {
            plot_pixel(x + j, y + i, 0);
        }
    }
}

//green tower
void plot_image_tower1(int x, int y) {
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            plot_pixel(x + j, y + i, tower1[i * 40 + j]);
        }
    }
}

void erase_image_tower1(int x, int y) {
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            plot_pixel(x + j, y + i, 0);
        }
    }
}

//monster
void plot_image_monster(int x, int y) {
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            plot_pixel(x + j, y + i, monster[i * 20 + j]);
        }
    }
}

void erase_image_monster(int x, int y) {
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            plot_pixel(x + j, y + i, 0);
        }
    }
}

//plot pixel function
void plot_pixel(int x, int y, short int color) {
    volatile short int *pixel_ptr =
        (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_ptr = color;
  }
  