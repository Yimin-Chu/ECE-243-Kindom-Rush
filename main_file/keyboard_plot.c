#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "address_map_nios_v.h" // 若无此头文件，可去掉
#include "math.h"
#include "string.h"
#include "ps2_keyboard.h"
#include "plot_image.h"


// 声明双缓冲区（每行 512 个 short int，320 像素 + 填充）
static short int Buffer1[240][512];
static short int Buffer2[240][512];

// ========== 移动盒子的结构体及相关 ==========
typedef struct
{
    int x, y;        // 盒子左上角坐标
    int dx, dy;      // 水平和垂直移动方向
    short int color; // 盒子颜色
} Box;

#define NUM_BOXES 5
#define BOX_SIZE 10 // 盒子尺寸: 10x10
static Box boxes[NUM_BOXES];

// 预定义的颜色（示例）
static short int colors[NUM_BOXES] = {
    0xF800, // 红
    0x07E0, // 绿
    0x001F, // 蓝
    0xFFFF, // 白
    0xF81F  // 粉
};

// 用“帧计数”模拟每过多少帧出现一个盒子
#define FRAMES_PER_BOX 60

// ========================================
// 函数声明
// ========================================
static void wait_for_vsync(void);
static void draw_line(int x0, int y0, int x1, int y1, short int color);
static void draw_filled_box(int x, int y, int width, int height, short int color);


// 用于记录绘制过的像素坐标
typedef struct
{
    int x;
    int y;
} Pixel;

#define MAX_DRAWN_PIXELS 5000
Pixel drawnPixels1[MAX_DRAWN_PIXELS]; // 针对 Buffer1
int drawnPixelCount1 = 0;
Pixel drawnPixels2[MAX_DRAWN_PIXELS]; // 针对 Buffer2
int drawnPixelCount2 = 0;
// 绘制一个像素并记录坐标
void plot_dynamic_pixel(int x, int y, short int color)
{
    volatile short int *pixel_addr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;

    // 根据当前缓冲区记录绘制的像素
    if (pixel_buffer_start == (int)&Buffer1)
    {
        if (drawnPixelCount1 < MAX_DRAWN_PIXELS)
        {
            drawnPixels1[drawnPixelCount1].x = x;
            drawnPixels1[drawnPixelCount1].y = y;
            drawnPixelCount1++;
        }
    }
    else if (pixel_buffer_start == (int)&Buffer2)
    {
        if (drawnPixelCount2 < MAX_DRAWN_PIXELS)
        {
            drawnPixels2[drawnPixelCount2].x = x;
            drawnPixels2[drawnPixelCount2].y = y;
            drawnPixelCount2++;
        }
    }
}

// 清除当前后缓冲区上上帧绘制的像素（擦成棕色）
void clear_drawn_pixels()
{
    int i;
    if (pixel_buffer_start == (int)&Buffer1)
    {
        for (i = 0; i < drawnPixelCount1; i++)
        {
            int x = drawnPixels1[i].x;
            int y = drawnPixels1[i].y;
            volatile short int *pixel_addr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
            *pixel_addr = 0x8A22; // 棕色
        }
        drawnPixelCount1 = 0;
    }
    else if (pixel_buffer_start == (int)&Buffer2)
    {
        for (i = 0; i < drawnPixelCount2; i++)
        {
            int x = drawnPixels2[i].x;
            int y = drawnPixels2[i].y;
            volatile short int *pixel_addr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
            *pixel_addr = 0x8A22;
        }
        drawnPixelCount2 = 0;
    }
}

// ========================================
// 主函数
// ========================================
int main(void)
{

    // 1) 初始化 PS/2 键盘
    initPS2Keyboard();

    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

    // ========== 1) Single Buffer: 显示 Intro ==========
    // 让 "前端" = Buffer1
    *pixel_ctrl_ptr = (int)&Buffer1[0];
    pixel_buffer_start = *pixel_ctrl_ptr; // 现在前端 = Buffer1

    // 3) 先绘制 intro 画面
    plot_image_intro(0, 0);

    // 此时屏幕已显示 intro（单缓冲）
    // 不再进行任何后端指针设置，不做双缓冲交换

    // ========== 2) 等待用户按下 Enter ==========

    while (1)
    {
        char scancode = readPS2ScanCode();
        if (scancode == 0x5A)
        {          // Enter = 0x5A
            break; // 跳出，开始双缓冲
        }
    }

    erase_image_intro(0,0);

    // 第一步：初始化“双缓冲”——让前端先指向 Buffer1
    // ======= 1) 初始化: 让前端指向 Buffer1 =======
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1[0];
    wait_for_vsync();                     // 交换生效
    pixel_buffer_start = *pixel_ctrl_ptr; // 现在“前端”=Buffer1

    // ======= 2) 在“前端(=Buffer1)”里绘制背景+塔 =======
    plot_image_game(0, 0);
    plot_image_tower1(100, 200);

    // ... 如果有其他静态元素，也在这儿画

    // ======= 3) 让后端指向 Buffer2 并同样绘制 =======
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2[0];
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // 现在“后端”=Buffer2

    // 在“后端(=Buffer2)”里也绘制背景+塔
    plot_image_background(0, 0);


    // ========== 初始化盒子 ==========
    // 初始都放在坐标 (280 - BOX_SIZE - 2, 80 - BOX_SIZE - 2) 等位置，
    // dx = -1, dy = 0
    for (int i = 0; i < NUM_BOXES; i++)
    {
        boxes[i].x = 280 - BOX_SIZE - 2;
        boxes[i].y = 20 - BOX_SIZE - 2;
        boxes[i].dx = -1;
        boxes[i].dy = 0;
        boxes[i].color = colors[i];
    }

    int num_draw_box = 0; // 当前实际出现的盒子数
    int frame_count = 0;  // 帧计数

    while (1)
    {
        // 后端缓冲区（即将绘制到的地址）
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        clear_drawn_pixels();

        // 2) 让新的盒子出现（类似每 60 帧出现一个盒子）
        frame_count++;
        if ((frame_count >= FRAMES_PER_BOX) && (num_draw_box < NUM_BOXES))
        {
            num_draw_box++;
            frame_count = 0;
        }

        // 3) 绘制已经出现的盒子
        for (int i = 0; i < num_draw_box; i++)
        {
            draw_filled_box(boxes[i].x, boxes[i].y, BOX_SIZE, BOX_SIZE, boxes[i].color);
        }

        // 4) 绘制相邻盒子连线
        /*for (int i = 0; i < num_draw_box - 1; i++)
        {
            int x1 = boxes[i].x + BOX_SIZE / 2;
            int y1 = boxes[i].y + BOX_SIZE / 2;
            int x2 = boxes[i + 1].x + BOX_SIZE / 2;
            int y2 = boxes[i + 1].y + BOX_SIZE / 2;
            draw_line(x1, y1, x2, y2, 0xFFFF); // 白线
        }*/

        // 5) 更新盒子移动坐标
        for (int i = 0; i < num_draw_box; i++)
        {
            // 根据需求设置转弯逻辑
            if (boxes[i].x == 280 - BOX_SIZE - 2 && boxes[i].y == 20 - BOX_SIZE - 2)
            {
                boxes[i].dy = 0;
                boxes[i].dx = -1;
            }
            else if (boxes[i].y == 20 - BOX_SIZE - 2 && boxes[i].x == 240 - BOX_SIZE - 2)
            {
                boxes[i].dy = 1;
                boxes[i].dx = 0;
            }
            else if (boxes[i].y == 200 - BOX_SIZE - 2 && boxes[i].x == 240 - BOX_SIZE - 2)
            {
                boxes[i].dy = 0;
                boxes[i].dx = -1;
            }
            else if (boxes[i].y == 200 - BOX_SIZE - 2 && boxes[i].x == 150 - BOX_SIZE - 2)
            {
                boxes[i].dy = -1;
                boxes[i].dx = 0;
            }
            else if (boxes[i].y == 20 - BOX_SIZE - 2 && boxes[i].x == 150 - BOX_SIZE - 2)
            {
                boxes[i].dy = 0;
                boxes[i].dx = -1;
            }
            else if (boxes[i].y == 20 - BOX_SIZE - 2 && boxes[i].x == 80 - BOX_SIZE - 2)
            {
                boxes[i].dy = 1;
                boxes[i].dx = 0;
            }
            else if (boxes[i].y == 200 - BOX_SIZE - 2 && boxes[i].x == 80 - BOX_SIZE - 2)
            {
                boxes[i].dy = 0;
                boxes[i].dx = 0; // 到达终点，停止
            }

            // 移动
            boxes[i].x += boxes[i].dx;
            boxes[i].y += boxes[i].dy;

            // 如果还需边界判定，可自行加逻辑
            if (boxes[i].x < 0)
            {
                boxes[i].x = 0;
                boxes[i].dx = 0;
            }
        }

        // 6) 等待 vsync (垂直同步) 完成缓冲区交换
        wait_for_vsync();
    }

    return 0;
}

// ========================================
// 辅助函数实现
// ========================================

// 等待垂直同步
static void wait_for_vsync(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    *pixel_ctrl_ptr = 1; // 发出交换缓冲区请求
    while ((*(pixel_ctrl_ptr + 3)) & 0x01)
        ; // 等待 S 位清零
    // 完成后, 前端与后端已交换
}



// Bresenham 画线
static void draw_line(int x0, int y0, int x1, int y1, short int color)
{
    bool is_steep = (abs(y1 - y0) > abs(x1 - x0));
    if (is_steep)
    {
        int temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
    }
    if (x0 > x1)
    {
        int temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int y_step = (y0 < y1) ? 1 : -1;

    for (int x = x0; x <= x1; x++)
    {
        if (is_steep)
            plot_dynamic_pixel(y, x, color);
        else
            plot_dynamic_pixel(x, y, color);
        error += deltay;
        if (error > 0)
        {
            y += y_step;
            error -= deltax;
        }
    }
}

// 画填充矩形(盒子)
static void draw_filled_box(int x, int y, int width, int height, short int color)
{
    for (int row = y; row < y + height; row++)
    {
        for (int col = x; col < x + width; col++)
        {
            plot_dynamic_pixel(col, row, color);
        }
    }
}

