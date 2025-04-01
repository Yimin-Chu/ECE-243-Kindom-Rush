#include "monster_moving.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


void update_monster_position(int *x, int *y, int *dx, int *dy) {
    // 根据当前坐标来判断是否需要转向
    // 下面是“示例转向”，你需要结合自己棕色道路的具体坐标去改

    // 1) 向上
    if (*x == 49 && *y == 180) {
        *dx = 0;  // 向右
        *dy = -1;
    }
    // 2) 当 x 到达 80 时，向上移动
    else if (*x == 49 && *y == 27) {
        *dx = 1; // 向右
        *dy = 0; // 向上
    }
    // 3) 当 y 到达 20 时，向右移动
    else if (*x == 122 && *y == 27) {
        *dx = 0;
        *dy = 1; 
    }
    else if( *x == 122 && *y == 183) {
        *dx = 1; // 向右
        *dy = 0; // 向下
    }
    else if( *x == 198 && *y == 183) {
        *dx = 0; // 向右
        *dy = -1; // 向上
    }
    else if(*x == 198 && *y == 27){
        *dx = 1; // 向右
        *dy = 0; // 向下
    }

    //delay
    for(int i = 0; i < 500000; i++); // 延时循环
    // 4) 移动坐标
    *x += *dx;
    *y += *dy;
}