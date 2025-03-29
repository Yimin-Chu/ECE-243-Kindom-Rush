#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// 全局像素缓冲区指针
volatile int pixel_buffer_start;

// 定义双缓冲区（每行 512 个 short int，320 像素加上填充）
short int Buffer1[240][512];
short int Buffer2[240][512];

// 定义移动盒子的结构体
typedef struct
{
    int x, y;        // 盒子左上角坐标
    int dx, dy;      // 水平和垂直移动方向
    short int color; // 盒子颜色
} Box;

#define NUM_BOXES 5
#define BOX_SIZE 10 // 盒子尺寸 10x10 像素

Box boxes[NUM_BOXES];

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
void plot_pixel(int x, int y, short int color)
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

// 清除当前后缓冲区上上帧绘制的像素（擦成黑色）
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
            *pixel_addr = 0x0000; // 黑色
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
            *pixel_addr = 0x0000;
        }
        drawnPixelCount2 = 0;
    }
}

// 用 Bresenham 算法绘制一条直线
void draw_line(int x0, int y0, int x1, int y1, short int color)
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
            plot_pixel(y, x, color);
        else
            plot_pixel(x, y, color);
        error += deltay;
        if (error > 0)
        {
            y += y_step;
            error -= deltax;
        }
    }
}

// 绘制一个填充矩形（盒子）
void draw_filled_box(int x, int y, int width, int height, short int color)
{
    for (int row = y; row < y + height; row++)
    {
        for (int col = x; col < x + width; col++)
        {
            plot_pixel(col, row, color);
        }
    }
}

// 等待垂直同步，完成缓冲区交换
void wait_for_vsync()
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    *pixel_ctrl_ptr = 1; // 请求交换缓冲区（vsync）
    // 等待状态寄存器中 S 位（bit 0）清零
    while (*(pixel_ctrl_ptr + 3) & 0x01)
        ;
}

int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    int i;

    // 双缓冲设置：
    // 将前端缓冲区设置为 Buffer1。
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
    wait_for_vsync(); // 交换缓冲区，此时前端为 Buffer1
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_drawn_pixels();

    // 将后端缓冲区设置为 Buffer2。
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_drawn_pixels();

    // 初始化随机种子（若只想固定方向，不一定需要随机数，可视需要保留或删除）
    srand(time(0));

    // 为盒子预定义颜色 (只需要 5 个颜色即可)
    short int colors[NUM_BOXES] = {
        0xF800, // 红
        0x07E0, // 绿
        0x001F, // 蓝
        0xFFFF, // 白
        0xF81F  // 粉
    };

    // ================
    // 初始化：右侧竖直排布
    // ================
    for (i = 0; i < NUM_BOXES; i++)
    {
        // 初始 x：屏幕最右端附近
        boxes[i].x = 320 - BOX_SIZE - 2;
        // 竖直方向上，从中间往上下排布
        boxes[i].y = 120 - (NUM_BOXES * (BOX_SIZE + 5)) / 2 + i * (BOX_SIZE + 5);

        // 只从右往左移动
        boxes[i].dx = -1;
        boxes[i].dy = 0;

        // 指定颜色
        boxes[i].color = colors[i];
    }

    while (1)
    {
        // 仅擦除上帧在当前后缓冲区上绘制的像素
        clear_drawn_pixels();

        // 绘制每个填充盒子
        for (i = 0; i < NUM_BOXES; i++)
        {
            draw_filled_box(boxes[i].x, boxes[i].y, BOX_SIZE, BOX_SIZE, boxes[i].color);
        }

        // 绘制连接相邻盒子中心的线条，形成链条
        for (i = 0; i < NUM_BOXES - 1; i++)
        {
            int x1 = boxes[i].x + BOX_SIZE / 2;
            int y1 = boxes[i].y + BOX_SIZE / 2;
            int x2 = boxes[i + 1].x + BOX_SIZE / 2;
            int y2 = boxes[i + 1].y + BOX_SIZE / 2;
            draw_line(x1, y1, x2, y2, 0xFFFF); // 用白色绘制线条
        }

        // 更新盒子位置
        for (i = 0; i < NUM_BOXES; i++)
        {
            boxes[i].x += boxes[i].dx;
            boxes[i].y += boxes[i].dy;

            // 若只想让它们停留在屏幕内，不反弹：
            // 当盒子越过左边界，固定在 x=0，不再继续移动
            if (boxes[i].x < 0)
            {
                boxes[i].x = 0;
                // 若完全不想让它们反弹，可以直接让dx=0停止：
                boxes[i].dx = 0;
            }

            // 上下若不想移动，可以不做任何检测
            // 若需要限制它们在屏幕内，也可写：
            // if (boxes[i].y < 0) boxes[i].y = 0;
            // if (boxes[i].y > 240 - BOX_SIZE) boxes[i].y = 240 - BOX_SIZE;
        }

        // 等待垂直同步并交换前后缓冲区
        wait_for_vsync();
        // 更新 pixel_buffer_start 指向新的后缓冲区
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    return 0;
}
