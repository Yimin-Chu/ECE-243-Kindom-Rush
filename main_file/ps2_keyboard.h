#ifndef __PS2_KEYBOARD_H__
#define __PS2_KEYBOARD_H__

// 函数原型
void initPS2Keyboard(void);
char readPS2ScanCode(void);
int get_block_index_from_scan_code(char code);
#endif
