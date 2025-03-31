#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


//all the h file needed
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"

//coordinate for tower
typedef struct {
    int x;
    int y;
} Coordinate;

Coordinate block_positions[11] = {
    {4, 37},  // Block 1
    {4, 85},  // Block 2
    {4, 133},  // Block 3
    {74, 60},  // Block 4
    {74, 107},  // Block 5
    {74, 154},  // Block 6
    {147, 60}, // Block 7
    {147, 107}, // Block 8
    {220, 60}, // Block 9
    {220, 107}, // Block 10
    {220, 154}  // Block 11
};

extern int pixel_buffer_start;
extern char ps2_keyboard_code;  // 如果在别的 .c 里有定义，则这里用 extern
bool released = false;

int main(void) {
    // 1) 初始化 PS/2 键盘
    initPS2Keyboard();

    // // 2) 获取显存指针
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
    

    // 3) 绘制 intro 画面
    plot_image_intro(0, 0);

    // 4) 等待用户按下 Enter（扫描码 0x5A）
    char ps2_keyboard_code;
    while (1) {
        ps2_keyboard_code = readPS2ScanCode();
        if (ps2_keyboard_code == 0x5A) {
            // 切换画面
            plot_image_game(0, 0);
            break;
        }
    }

    // 5) 等待数字键输入，绘制塔
    while (1) {
        ps2_keyboard_code = readPS2ScanCode();

        if (ps2_keyboard_code == 0xF0) {
            released = true; // 表示下一个码是“释放键”
            continue;
        }

        if (!released) {
            int block_idx = get_block_index_from_scan_code(ps2_keyboard_code);
            if (block_idx >= 0 && block_idx < 11) {
                Coordinate pos = block_positions[block_idx];
                plot_image_tower1(pos.x, pos.y);
            }
        }

        released = false; // 重置释放状态
    }

    return 0;
}

