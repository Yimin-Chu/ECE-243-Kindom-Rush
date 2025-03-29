#ifndef __PS2_KEYBOARD_H__
#define __PS2_KEYBOARD_H__

// 全局变量声明（如果要在外部访问的话）
extern volatile char ps2_keyboard_code;

// 函数原型
void initPS2Keyboard(void);
char readPS2ScanCode(void);

#endif
