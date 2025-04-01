#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


//all the h file needed
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"
#include "monster_moving.h"

//coordinate for tower
typedef struct {
    int x;
    int y;
} Coordinate;

Coordinate block_positions[10] = {
    {7, 39},  // Block 1
    {7, 88},  // Block 2
    {7, 137},  // Block 3
    {80, 60},  // Block 4
    {80, 109},  // Block 5
    {80, 158},  // Block 6
    {155, 72}, // Block 7
    {155, 137}, // Block 8
    {235, 60}, // Block 9
    {235, 109}, // Block 10
};

extern int pixel_buffer_start;
extern char ps2_keyboard_code;  // 如果在别的 .c 里有定义，则这里用 extern
bool released = false;

int monster_x = 60;   // 根据你地图的“左下角”位置来设定
int monster_y = 190;  
int dx = 0, dy = 0;   // 初始方向为 (0,0)，进入移动逻辑后再赋值
bool monster_finished = false; // 是否已经到达终点

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

    // 5) 等待数字键输入，绘制塔
    while (1) {
        ps2_keyboard_code = readPS2ScanCode();

        if (ps2_keyboard_code == 0xF0) {
            released = true; // 表示下一个码是“释放键”
            continue;
        }

        if (!released) {
            int block_idx = get_block_index_from_scan_code(ps2_keyboard_code);
            if (block_idx >= 0 && block_idx < 10) {
                Coordinate pos = block_positions[block_idx];
                plot_image_tower1(pos.x, pos.y);
            }
        }

        released = false; // 重置释放状态

        if (!monster_finished) {
            // 先擦除上一帧的怪物
            erase_image_monster(monster_x, monster_y);

            // 更新坐标
            update_monster_position(&monster_x, &monster_y, &dx, &dy);

            // 判断是否抵达终点(示例：假设 x>300 就算到达右上角)
            // 具体终点可根据实际地图坐标而定
            if (monster_x > 280) {
                monster_finished = true;
            } else {
                // 重新绘制怪物
                plot_image_monster(monster_x, monster_y);
            }
        }
    }

    return 0;
}

