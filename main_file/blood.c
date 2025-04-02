#include "blood.h"
#include "plot_image.h"   // 提供 plot_pixel() 函数
#include "address_map_nios_v.h" // 如需要，定义颜色等

// 全局生命值，初始设为 5
int blood = 5;

// 定义生命值显示的位置（你可以根据实际情况调整）
#define BLOOD_X 307
#define BLOOD_Y 65

// 使用一个简单的 8x8 点阵字库绘制数字 0-9
// 每个数字由 8 行组成，每行 8 个像素，1 表示该位置需要绘制，0 表示空白
static const unsigned char digitPatterns[10][8] = {
    { 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00 }, // 0
    { 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00 }, // 1
    { 0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00 }, // 2
    { 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00 }, // 3
    { 0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00 }, // 4
    { 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00 }, // 5
    { 0x3C, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C, 0x00 }, // 6
    { 0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00 }, // 7
    { 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00 }, // 8
    { 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x3C, 0x00 }  // 9
};

// 绘制一个数字到 (x, y) 位置，使用固定颜色（例如白色 0xFFFF）
// 这里采用 8x8 的点阵，每个 bit 对应一个像素
void plot_digit(int x, int y, int digit) {
    if (digit < 0 || digit > 9) return;
    for (int row = 0; row < 8; row++) {
        unsigned char pattern = digitPatterns[digit][row];
        for (int col = 0; col < 8; col++) {
            // 判断当前 bit 是否为 1，注意最高位对应最左边的像素
            if (pattern & (1 << (7 - col))) {
                plot_pixel(x + col, y + row, 0xFFFF);  // 白色像素
            }
        }
    }
}

// 初始化血量并绘制初始生命值
void init_blood(void) {
    blood = 5;
    draw_blood();
}

// 绘制当前血量，假设血量为单个数字（0-9），若血量超过 9 需要扩展绘制方式
void draw_blood(void) {
    // 清除生命值显示区域
    // 此处假设背景为黑色 (0)，如果背景颜色不同请修改
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            plot_pixel(BLOOD_X + j, BLOOD_Y + i, 0);
        }
    }
    // 绘制数字
    plot_digit(BLOOD_X, BLOOD_Y, blood);
}

// 当怪物到达终点时调用，血量减1并重绘
void decrease_blood(void) {
    if (blood > 0) {
        blood--;
    }
    plot_digit(BLOOD_X, BLOOD_Y, blood); // 更新显示
}
