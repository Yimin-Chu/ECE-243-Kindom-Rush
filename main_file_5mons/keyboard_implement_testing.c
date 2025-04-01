#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// 包含所有必要的头文件
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"
#include "monster_moving.h"

// 坐标结构体（用于塔放置）
typedef struct {
    int x;
    int y;
} Coordinate;

// 预定义塔的位置（固定）
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

extern int pixel_buffer_start;
bool released = false;

// 定义一个数组记录每个塔是否放置，初始全部为 false
bool tower_placed[10] = { false };

// ========== 移动怪物的结构体及相关 ==========
typedef struct
{
    int x, y;        // 盒子左上角坐标
    int dx, dy;      // 水平和垂直移动方向
    short int color; // 盒子颜色
} mons;

#define NUM_BOXES 5
#define BOX_SIZE 10 // 盒子尺寸: 10x10
static mons boxes[NUM_BOXES];

// 预定义的颜色（示例）
static short int colors[NUM_BOXES] = {
    0xF800, // 红
    0x07E0, // 绿
    0x001F, // 蓝
    0xFFFF, // 白
    0xF81F  // 粉
};

int num_draw_box = 0; // 当前实际出现的盒子数
int frame_count = 0;  // 帧计数
bool monster_finished = false;

// 用“帧计数”模拟每过多少帧出现一个盒子
#define FRAMES_PER_BOX 40

int main(void) {
    // 1) 初始化 PS/2 键盘
    initPS2Keyboard();

    // 2) 获取显存控制器指针
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
    
    // 3) 显示 Intro 画面
    plot_image_intro(0, 0);

    // 4) 等待用户按下 Enter（扫描码 0x5A）
    char code;
    while (1) {
        code = readPS2ScanCode();
        if (code == 0x5A) {
            break;
        }
    }

    // 5) 初始化双缓冲：
    // 设置前缓冲区为 Buffer1 并调用 wait_for_vsync() 交换缓冲区，
    // 然后在前缓冲区绘制静态背景（包含棕色道路等）
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1[0];
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    plot_image_game(0, 0);  // 绘制静态背景

    // 设置后缓冲区为 Buffer2，并在后缓冲区预绘制静态背景
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2[0];
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    plot_image_game(0, 0);

    // 怪物初始位置及运动状态（根据你的棕色道路的起点调整）
    for (int i = 0; i < NUM_BOXES; i++)
    {
        boxes[i].x = 49;
        boxes[i].y = 180;
        boxes[i].dx = 0;
        boxes[i].dy = -1;
        boxes[i].color = colors[i];
    }

    // 6) 进入主循环：每帧先重绘背景，再绘制静态塔和动态怪物
    while (1) {
        // 将后缓冲区设为当前绘图目标
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        
        // --- 重绘静态背景 ---
        plot_image_game(0, 0);

        // 让新的怪物出现（类似每 60 帧出现一个盒子）
        frame_count++;
        if ((frame_count >= FRAMES_PER_BOX) && (num_draw_box < NUM_BOXES))
        {
            num_draw_box++;
            frame_count = 0;
        }

        // --- 绘制所有已放置的塔 ---
        for (int i = 0; i < 10; i++) {
            if (tower_placed[i]) {
                plot_image_tower1(block_positions[i].x, block_positions[i].y);
            }
        }

        // --- 动态元素：怪物运动 ---
        // 绘制已经出现的盒子
        for (int i = 0; i < num_draw_box && !monster_finished; i++)
        {
            update_monster_position(&boxes[i].x, &boxes[i].y, &boxes[i].dx, &boxes[i].dy);
            if (boxes[i].x > 260)
            { // 根据实际情况调整终点条件
                monster_finished = true;
            }
            else
            {
                plot_image_monster(boxes[i].x, boxes[i].y);
            }
        }
        
        
        // --- 处理键盘输入：塔的放置 ---
        // 使用非阻塞方式读取键盘扫描码，若没有新数据则返回 0
        code = readPS2ScanCode();
        if (code != 0) {
            if (code == 0xF0) {
                released = true;
            } else {
                if (!released) {
                    int block_idx = get_block_index_from_scan_code(code);
                    if (block_idx >= 0 && block_idx < 10) {
                        // 将对应塔标记为已放置，使其在后续帧中一直绘制
                        tower_placed[block_idx] = true;
                    }
                }
                released = false;
            }
        }
        
        // --- 交换缓冲区 ---
        wait_for_vsync();
        // 交换后，后缓冲区完整画面将显示到屏幕上，下一帧继续在新后缓冲区绘制
    }

    return 0;
}
