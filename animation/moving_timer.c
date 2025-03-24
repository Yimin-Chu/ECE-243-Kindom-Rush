#include <stdlib.h>
#include <stdbool.h>
#include <time.h>  // 可选，用于调试
#include <stdio.h> // 可选，用于调试

// --------------------------------------
// ========== 16-bit Interval Timer (在32位地址空间) ==========
// 参考你提供的截图：
// 0xFF202000 -> Status (16位, bits: RUN=0, TO=1, ITO=2, CONT=3, START=4, STOP=5, …)
// 0xFF202004 -> Control
// 0xFF202008 -> Counter start (low)
// 0xFF20200C -> Counter start (high)
// 0xFF202010 -> Counter snapshot (low)
// 0xFF202014 -> Counter snapshot (high)
// --------------------------------------
#define TIMER_BASE 0xFF202000

// 每个寄存器 16 位，但在地址空间中以 4 字节对齐
#define TIMER_STATUS (*(volatile unsigned short *)(TIMER_BASE + 0x0))
#define TIMER_CONTROL (*(volatile unsigned short *)(TIMER_BASE + 0x4))
#define TIMER_START_LO (*(volatile unsigned short *)(TIMER_BASE + 0x8))
#define TIMER_START_HI (*(volatile unsigned short *)(TIMER_BASE + 0xC))
#define TIMER_SNAP_LO (*(volatile unsigned short *)(TIMER_BASE + 0x10))
#define TIMER_SNAP_HI (*(volatile unsigned short *)(TIMER_BASE + 0x14))

// Status/Control位的掩码（与截图中一致）
#define BIT_RUN (1 << 0)   // bit0: RUN (只读, 1=正在运行)
#define BIT_TO (1 << 1)    // bit1: TO (溢出标志)
#define BIT_ITO (1 << 2)   // bit2: ITO (是否使能中断)
#define BIT_CONT (1 << 3)  // bit3: CONT (连续模式)
#define BIT_START (1 << 4) // bit4: START
#define BIT_STOP (1 << 5)  // bit5: STOP
// 其余位在截图中显示为 unused

// --------------------------------------
// 全局像素缓冲区指针及双缓冲区定义
// --------------------------------------
volatile int pixel_buffer_start;
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

// --------------------------------------
// 计时相关：利用硬件定时器做 1ms 周期溢出
// --------------------------------------
static unsigned int ms_counter = 0; // 全局毫秒计数

// ============ 硬件定时器初始化 ============
// 设置为每 1ms 溢出一次 (大约)，连续模式，启动
// 假设主频 50MHz => 1 个周期 20ns => 50000 周期 ~ 1ms
void init_hardware_timer(void)
{
    // 1) 先停止定时器
    TIMER_CONTROL = 0; // 写 0 即可，也可写 BIT_STOP
    TIMER_STATUS = 0;  // 写 0 清除 TO 等标志

    // 2) 装载值
    // 16 位定时器一次只能数到 65535
    // 这里设置 50000 => ~1ms
    TIMER_START_LO = 50000; // 低 16 位
    TIMER_START_HI = 0;     // 高 16 位 (若只需16位则设0)

    // 3) 设置连续模式 + 启动
    // CONT=1, START=1 => (1<<3) | (1<<4)
    TIMER_CONTROL = BIT_CONT | BIT_START;
}

// ============ 轮询硬件定时器溢出 ============
// 每次 TO=1 表示间隔 1ms 已到
// 主循环中调用本函数，检测到 TO=1 则 ms_counter++
void poll_timer_and_update_ms(void)
{
    if (TIMER_STATUS & BIT_TO) // 如果 TO=1
    {
        TIMER_STATUS = 0; // 写 0 清 TO
        ms_counter++;     // +1ms
    }
}

unsigned int get_ms_time(void)
{
    return ms_counter;
}

// --------------------------------------
// 以下为 VGA 绘制与双缓冲相关函数
// --------------------------------------

void plot_pixel(int x, int y, short int color)
{
    volatile short int *pixel_addr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;

    // 根据当前后缓冲区记录绘制的像素
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

void clear_drawn_pixels(void)
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
    else
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

// Bresenham 画线
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

// 填充矩形（盒子）
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
void wait_for_vsync(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    *pixel_ctrl_ptr = 1; // 请求交换前后缓冲区（vsync）
    while (*(pixel_ctrl_ptr + 3) & 0x01)
        ;
}

// --------------------------------------
// 主函数：利用上述 16-bit Interval Timer (32-bit地址映射)
// 实现“每秒多出现一个盒子，从右向左运动”
// --------------------------------------
int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    int i;

    // 1) 初始化视频双缓冲
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_drawn_pixels();

    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_drawn_pixels();

    // 2) 初始化硬件定时器
    init_hardware_timer();

    // 3) 初始化盒子
    // 为盒子预定义 5 种颜色
    short int colors[NUM_BOXES] = {
        0xF800, // 红
        0x07E0, // 绿
        0x001F, // 蓝
        0xFFFF, // 白
        0xF81F  // 粉
    };

    // 让盒子初始在屏幕右侧、竖直居中排布
    for (i = 0; i < NUM_BOXES; i++)
    {
        boxes[i].x = 320 - BOX_SIZE - 2;
        boxes[i].y = 120 - (NUM_BOXES * (BOX_SIZE + 5)) / 2 + i * (BOX_SIZE + 5);
        boxes[i].dx = -1; // 从右向左移动
        boxes[i].dy = 0;
        boxes[i].color = colors[i];
    }

    // 4) 主循环：每秒增加一个盒子
    unsigned int prev_second_count = 0;
    unsigned int activeCount = 0;

    while (1)
    {
        // a) 轮询定时器溢出：每次 TO=1 => +1ms
        poll_timer_and_update_ms();

        // b) 判断是否过了新的一秒
        unsigned int current_ms = get_ms_time();
        unsigned int current_second = current_ms / 1000; // 1000ms = 1s
        if (current_second > prev_second_count)
        {
            prev_second_count = current_second;
            if (activeCount < NUM_BOXES)
            {
                activeCount++;
            }
        }

        // c) 清除上一帧像素
        clear_drawn_pixels();

        // d) 绘制 activeCount 个盒子
        for (i = 0; i < activeCount; i++)
        {
            draw_filled_box(boxes[i].x, boxes[i].y, BOX_SIZE, BOX_SIZE, boxes[i].color);
        }

        // 绘制连接相邻盒子中心的线条
        for (i = 0; i < activeCount - 1; i++)
        {
            int x1 = boxes[i].x + BOX_SIZE / 2;
            int y1 = boxes[i].y + BOX_SIZE / 2;
            int x2 = boxes[i + 1].x + BOX_SIZE / 2;
            int y2 = boxes[i + 1].y + BOX_SIZE / 2;
            draw_line(x1, y1, x2, y2, 0xFFFF);
        }

        // e) 更新盒子位置，碰到左边界就停
        for (i = 0; i < activeCount; i++)
        {
            boxes[i].x += boxes[i].dx;
            boxes[i].y += boxes[i].dy;
            if (boxes[i].x < 0)
            {
                boxes[i].x = 0;
                boxes[i].dx = 0;
            }
        }

        // f) 等待垂直同步并交换前后缓冲区
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    return 0;
}
