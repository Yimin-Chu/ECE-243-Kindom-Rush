#ifndef __PLOT_IMAGE_H__
#define __PLOT_IMAGE_H__


// 如果你打算把图像数组定义在别的文件，也可以在这里 extern

extern short unsigned int intro[76800];
extern short unsigned int tower1[1225];
extern short unsigned int game[76800];
extern short unsigned int monster[625];

void plot_image_game(int x, int y);
void erase_image_game(int x, int y);

void plot_image_intro(int x, int y);
void erase_image_intro(int x, int y);

void plot_image_tower1(int x, int y);
void erase_image_tower1(int x, int y);

void plot_image_monster(int x, int y);
void erase_image_monster(int x, int y);

void plot_pixel(int x, int y, short int color);

#endif
