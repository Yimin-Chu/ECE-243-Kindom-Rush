#include "monster_moving.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


void update_monster_position(int *x, int *y, int *dx, int *dy) {
    // 根据当前坐标来判断是否需要转向
    // 下面是“示例转向”，你需要结合自己棕色道路的具体坐标去改

    // 1) 如果怪物在 (20,220) 刚开始，就往右移动
    if (*x == 60 && *y == 190) {
        *dx = 0;  // 向右
        *dy = -1;
    }
    // 2) 当 x 到达 80 时，向上移动
    else if (*x == 60 && *y == 36) {
        *dx = 1; // 向右
        *dy = 0; // 向上
    }
    // 3) 当 y 到达 20 时，向右移动
    else if (*x == 133 && *y == 36) {
        *dx = 0;
        *dy = 1; 
    }
    else if( *x == 133 && *y == 190) {
        *dx = 1; // 向右
        *dy = 0; // 向下
    }
    else if( *x == 210 && *y == 190) {
        *dx = 0; // 向右
        *dy = -1; // 向上
    }
    else if(*x == 210 && *y == 36){
        *dx = 1; // 向右
        *dy = 0; // 向下
    }

    // 4) 移动坐标
    *x += *dx;
    *y += *dy;
}