#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//all the h file needed
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"

// 全局变量（如果你在别的 .c 文件里定义了，这里就用 extern 声明即可）
extern int pixel_buffer_start;
extern char ps2_keyboard_code;  // 如果在别的 .c 里有定义，则这里用 extern

int main(void) {
    // 1) 初始化 PS/2 键盘
    initPS2Keyboard();

    // 2) 获取显存指针
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;

    // 3) 先绘制 intro 画面
    plot_image_intro(0, 0);

    // 4) 等待用户按下 Enter（PS/2 扫描码 0x5A）
    while (1) {
        // 每次 readPS2ScanCode() 会阻塞，直到读到新的扫描码
        ps2_keyboard_code = readPS2ScanCode();

        // 判断是否是 Enter 键的扫描码
        if (ps2_keyboard_code == 0x5A) {
            // 5) 绘制 background 画面
            plot_image_background(0, 0);
            break;  // 若只想切换一次画面，可以 break；否则可去掉
        }
    }

    // 6) 后续逻辑（可选）
    

    return 0;
}


