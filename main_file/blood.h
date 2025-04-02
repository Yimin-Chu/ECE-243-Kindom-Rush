#ifndef BLOOD_H
#define BLOOD_H

// 当前生命值，全局变量（初始为 5）
extern int blood;

// 初始化生命值（设为5）并绘制初始生命值
void init_blood(void);

// 绘制当前生命值到屏幕上（例如右上角）
void draw_blood(void);

// 当怪物到达终点时调用，生命值减1并重绘
void decrease_blood(void);

void plot_digit(int x, int y, int digit);

#endif // BLOOD_H
