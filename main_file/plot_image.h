#ifndef __PLOT_IMAGE_H__
#define __PLOT_IMAGE_H__

// 声明全局变量（在 plot_image.c 中定义）
extern volatile pixel_buffer_start;

// 如果你打算把图像数组定义在别的文件，也可以在这里 extern
extern short unsigned int background[76800];
extern short unsigned int tower1[42*26];
extern short unsigned int tower1[42*26];
extern short unsigned int intro[240*320];

// 声明函数
void plot_static_pixel(int x, int y, short int color);
void plot_image_background(int x, int y);
void erase_image_background(int x, int y);
void plot_image_tower1(int x, int y);
void erase_image_tower1(int x, int y);
void plot_image_tower2(int x, int y);
void erase_image_tower2(int x, int y);
void plot_image_intro(int x, int y);
void erase_image_intro(int x, int y);

#endif
