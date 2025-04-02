#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h> // 用于 sqrtf()

// 包含所有必要的头文件
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"
#include "monster_moving.h"
#include "coin.h" // 包含金币相关函数
#include "blood.h" // 包含生命值相关函数

// 坐标结构体（用于塔放置）
typedef struct
{
    int x;
    int y;
} Coordinate;

// 预定义塔的位置（固定）
Coordinate block_positions[10] = {
    {7, 39},    // Block 1
    {7, 88},    // Block 2
    {7, 137},   // Block 3
    {80, 60},   // Block 4
    {80, 109},  // Block 5
    {80, 158},  // Block 6
    {155, 72},  // Block 7
    {155, 137}, // Block 8
    {235, 60},  // Block 9
    {235, 109}  // Block 10
};

extern int pixel_buffer_start;
bool released = false;

//blood
extern int blood; // 生命值变量，定义在 blood.c 中

//coin
extern int coin; // 金币变量，定义在 coin.c 中

// 记录每个塔是否放置，初始全部为 false
bool tower_placed[10] = {false};
// 塔射击计时器（每个塔独立计时）
#define FRAMES_PER_SHOT 60 // 60帧约1秒
int tower_shoot_counter[10] = {0};

// ========== 怪兽（盒子）相关 ==========

typedef struct
{
    int x, y;        // 盒子左上角坐标
    int dx, dy;      // 水平和垂直移动方向
    short int color; // 盒子颜色
    bool alive;      // 是否存活（true：存活，false：死亡或走出终点）
    bool killed;
    bool finished;
} mons;

#define NUM_BOXES 5
#define BOX_SIZE 10 // 盒子尺寸: 10x10
static mons boxes[NUM_BOXES];

// 预定义怪兽颜色（示例）
static short int colors[NUM_BOXES] = {
    0xF800, // 红
    0x07E0, // 绿
    0x001F, // 蓝
    0xFFFF, // 白
    0xF81F  // 粉
};

int num_draw_box = 0; // 当前实际出现的怪兽盒子数
int frame_count = 0;  // 帧计数


//有效距离 子弹
#define Tower_RANGE 100 // 子弹有效射程

//怪物的生命值
int monster_health[NUM_BOXES];

void init_monster(mons *box, int index)
{
    box->x = 49; // 初始位置
    box->y = 180;
    box->dx = 0; // 水平移动方向
    box->dy = -1; // 垂直移动方向
    box->color = colors[index]; // 颜色
    box->alive = true; // 初始存活状态
    box->killed = false; // 初始未被击杀状态
    box->finished = false; // 初始未完成状态
    monster_health[index] = 2; // 生命值
}

//重置
void reset_all_monsters(void)
{
    for (int i = 0; i < NUM_BOXES; i++)
    {
        init_monster(&boxes[i], i);
    }
    num_draw_box = 0;
    frame_count = 0;
}

// 更新怪兽位置
// void update_monster_position(mons* box)
// {
//     for (int i = 0; i < num_draw_box; i++)
//     {
//         boxes[i].x += boxes[i].dx;
//         boxes[i].y += boxes[i].dy;
//         if(boxes[i].x >= 252)
//         {
//             boxes[i].alive = false;
//             boxes[i].finished = true; // 标记为完成
//             boxes[i].x=0;
//         }
//     }
// }


// ========== 子弹相关 ==========

#define MAX_BULLETS 20

typedef struct
{
    float x, y;               // 子弹当前坐标
    float target_x, target_y; // 目标怪兽中心位置
    float speed;              // 子弹速度
    bool active;              // 子弹是否处于激活状态
    int tower_index;          // 发射该子弹的塔的索引
    int target_monster_index; // 目标怪兽在 boxes 数组中的索引
} Bullet;

static Bullet bullets[MAX_BULLETS]; // 全局子弹数组

/*
 * spawn_bullet:
 * 从指定塔 (tower_index) 发射一颗子弹，
 * 目标为当前第一个存活怪兽（通过 get_first_alive_monster() 获取）。
 */
void spawn_bullet(int tower_index) {
    int target_index = -1;
    // 遍历所有怪兽，选择一个在有效射程内且存活的
    for (int i = 0; i < num_draw_box; i++) {
        if (boxes[i].alive) {
            // 计算塔中心（塔坐标 +30, +30）
            int tower_cx = block_positions[tower_index].x + 30;
            int tower_cy = block_positions[tower_index].y + 30;
            // 目标怪兽中心
            int monster_cx = boxes[i].x + BOX_SIZE/2;
            int monster_cy = boxes[i].y + BOX_SIZE/2;
            int dx = monster_cx - tower_cx;
            int dy = monster_cy - tower_cy;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist <= Tower_RANGE) {
                target_index = i;
                break;
            }
        }
    }
    if (target_index == -1)
        return; // 没有有效目标则不发射

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].tower_index = tower_index;
            bullets[i].target_monster_index = target_index;
            // 子弹起始位置：塔中心
            bullets[i].x = block_positions[tower_index].x + 30;
            bullets[i].y = block_positions[tower_index].y + 30;
            bullets[i].speed = 5.0f;  // 可调
            // 目标为目标怪兽中心
            bullets[i].target_x = boxes[target_index].x + BOX_SIZE/2;
            bullets[i].target_y = boxes[target_index].y + BOX_SIZE/2;
            break;
        }
    }
}

// update_bullets: 每帧更新激活的子弹，并判断是否命中目标
void update_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            int target_index = bullets[i].target_monster_index;
            // 如果目标不存活，则尝试重新选择
            if (target_index >= num_draw_box || !boxes[target_index].alive) {
                target_index = -1;
                for (int j = 0; j < num_draw_box; j++) {
                    if (boxes[j].alive) {
                        // 检查距离是否在射程内
                        int tower_cx = block_positions[bullets[i].tower_index].x + 30;
                        int tower_cy = block_positions[bullets[i].tower_index].y + 30;
                        int monster_cx = boxes[j].x + BOX_SIZE/2;
                        int monster_cy = boxes[j].y + BOX_SIZE/2;
                        int dx = monster_cx - tower_cx;
                        int dy = monster_cy - tower_cy;
                        float dist = sqrtf(dx*dx + dy*dy);
                        if (dist <= Tower_RANGE) {
                            target_index = j;
                            break;
                        }
                    }
                }
                if (target_index == -1) {
                    bullets[i].active = false;
                    continue;
                }
                bullets[i].target_monster_index = target_index;
            }
            // 更新目标位置
            bullets[i].target_x = boxes[target_index].x + BOX_SIZE/2;
            bullets[i].target_y = boxes[target_index].y + BOX_SIZE/2;
            // 计算距离
            float dx = bullets[i].target_x - bullets[i].x;
            float dy = bullets[i].target_y - bullets[i].y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 5.0f) {  // 命中目标
                bullets[i].active = false;
                // 扣除怪兽健康值
                monster_health[target_index]--;
                if (monster_health[target_index] <= 0) {
                    boxes[target_index].alive = false;
                    boxes[target_index].killed = true;
                    boxes[target_index].finished = false;
                }
                continue;
            }
            // 归一化方向并更新位置
            float nx = dx / dist;
            float ny = dy / dist;
            bullets[i].x += nx * bullets[i].speed;
            bullets[i].y += ny * bullets[i].speed;
            // 绘制子弹（调用动态像素绘制函数）
            plot_dynamic_pixel((int)bullets[i].x, (int)bullets[i].y, 0xFFFF);
        }
    }
}


int main(void)
{
    // 1) 初始化 PS/2 键盘
    initPS2Keyboard();

    display_3digits_on_HEX(coin); // 初始化显示 40 个金币


    // 2) 获取显存控制器指针
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;

    // 3) 显示 Intro 画面
    plot_image_intro(0, 0);

    // 4) 等待用户按下 Enter（扫描码 0x5A）
    char code;
    while (1)
    {
        code = readPS2ScanCode();
        if (code == 0x5A)
        {
            break;
        }
    }

    // 5) 初始化双缓冲
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1[0];
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    plot_image_game(0, 0);
    plot_digit(302, 62, blood); // 绘制生命值

    *(pixel_ctrl_ptr + 1) = (int)&Buffer2[0];
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    plot_image_game(0, 0);
    plot_digit(302, 62, blood); // 绘制生命值

    //初始化怪兽
    for(int i = 0; i < NUM_BOXES; i++)
    {
        init_monster(&boxes[i], i);
    }
    num_draw_box = 0; // 当前实际出现的怪兽数
    frame_count = 0; // 帧计数

    // 主循环：每帧更新背景、怪兽、塔、子弹等
    while (1)
    {
        // 设置后缓冲区为当前绘图目标
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        // --- 重绘静态背景 ---
        plot_image_game(0, 0);
        plot_digit(302, 62, blood); // 绘制生命值
        display_3digits_on_HEX(coin); // 显示金币数量

        // 控制怪兽盒子出现（每 FRAMES_PER_BOX 帧出现一个）
        frame_count++;
        if ((frame_count >= FRAMES_PER_SHOT) && (num_draw_box < NUM_BOXES))
        {
            num_draw_box++;
            frame_count = 0;
        }

        // --- 绘制所有已放置的塔 ---
        for (int i = 0; i < 10; i++)
        {
            if (tower_placed[i])
            {
                plot_image_tower1(block_positions[i].x, block_positions[i].y);
            }
        }

        // --- 动态元素：怪兽运动 ---
        for (int i = 0; i < num_draw_box; i++)
        {
            if (boxes[i].alive)
            {
                update_monster_position(&boxes[i].x, &boxes[i].y, &boxes[i].dx, &boxes[i].dy);
                // 如果怪兽到达终点，则标记为死亡，但不影响其他怪兽
                if(boxes[i].alive)
                {
                    plot_image_monster(boxes[i].x, boxes[i].y);
                }
            }
            if (boxes[i].x >= 252)
            {
                boxes[i].alive = false;
                boxes[i].finished = true; // 标记为完成
                boxes[i].x=0;
            }
            if(boxes[i].finished)
            {
                decrease_blood(); // 生命值减少
                boxes[i].finished = false; // 重置完成状态
                init_monster(&boxes[i], i); // 重置怪兽
            }
            if (boxes[i].killed)
            {
                add_coin(); // 击杀怪兽后增加金币
                boxes[i].killed = false; // 重置击杀状态
                //重置
                init_monster(&boxes[i], i); // 重置怪兽
            }
        }

        // --- 处理键盘输入：塔的放置（仅初次放置） ---
        code = readPS2ScanCode();
        if (code != 0) {
            if (code == 0xF0) {
                released = true;
            } else {
                if (!released) {
                    int block_idx = get_block_index_from_scan_code(code);
                    if (block_idx >= 0 && block_idx < 10) {
                        // 将对应塔标记为已放置，使其在后续帧中一直绘制
                        if(can_place_tower()) {
                            tower_placed[block_idx] = true;
                            spend_coin(); // 扣除金币
                        }
                        //tower_placed[block_idx] = true; // 放置塔
                    }   
                }
                released = false;
            }
        }

        

        // --- 每个已放置塔每过1秒（FRAMES_PER_SHOT 帧）发射一颗子弹 ---
        for (int i = 0; i < 10; i++)
        {
            if (tower_placed[i])
            {
                tower_shoot_counter[i]++;
                if (tower_shoot_counter[i] >= FRAMES_PER_SHOT)
                {
                    // 发射子弹时，目标选择当前第一个存活怪兽
                    spawn_bullet(i);
                    tower_shoot_counter[i] = 0;
                }
            }
        }

        // --- 更新并绘制子弹 ---
        update_bullets();

        // --- 可选：调用 shoot_bullet() 绘制调试用连线 ---
        //      shoot_bullet();

        // --- 交换缓冲区 ---
        wait_for_vsync();
    }

    if (blood==0){
        plot_image_over(0, 0); // 绘制游戏结束画面
    }

    return 0;
}

/*#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// 包含所有必要的头文件
#include "address_map_nios_v.h"
#include "ps2_keyboard.h"
#include "plot_image.h"
#include "monster_moving.h"

// 坐标结构体（用于塔放置）
typedef struct {
    int x;
    int y;
} Coordinate;

// 预定义塔的位置（固定）
Coordinate block_positions[10] = {
    {7, 39},    // Block 1
    {7, 88},    // Block 2
    {7, 137},   // Block 3
    {80, 60},   // Block 4
    {80, 109},  // Block 5
    {80, 158},  // Block 6
    {155, 72},  // Block 7
    {155, 137}, // Block 8
    {235, 60},  // Block 9
    {235, 109}  // Block 10
};

extern int pixel_buffer_start;
bool released = false;

// 定义一个数组记录每个塔是否放置，初始全部为 false
bool tower_placed[10] = { false };

// ========== 移动怪物的结构体及相关 ==========
typedef struct
{
    int x, y;        // 盒子左上角坐标
    int dx, dy;      // 水平和垂直移动方向
    short int color; // 盒子颜色
} mons;

#define NUM_BOXES 5
#define BOX_SIZE 10 // 盒子尺寸: 10x10
static mons boxes[NUM_BOXES];

// 预定义的颜色（示例）
static short int colors[NUM_BOXES] = {
    0xF800, // 红
    0x07E0, // 绿
    0x001F, // 蓝
    0xFFFF, // 白
    0xF81F  // 粉
};

int num_draw_box = 0; // 当前实际出现的盒子数
int frame_count = 0;  // 帧计数
bool monster_finished = false;

// 用“帧计数”模拟每过多少帧出现一个盒子
#define FRAMES_PER_BOX 40

// void shoot_bullet(void)
// {
//     short int bullet_color = 0xFFFF; // 选用白色作为子弹颜色

//     // --- 绘制子弹连线 1：连接所有怪物盒子 ---
//     if (num_draw_box > 0)
//     {
//         // 从第一个盒子中心开始
//         int x0 = boxes[0].x + BOX_SIZE / 2;
//         int y0 = boxes[0].y + BOX_SIZE / 2;
//         for (int i = 1; i < num_draw_box; i++)
//         {
//             int x1 = boxes[i].x + BOX_SIZE / 2;
//             int y1 = boxes[i].y + BOX_SIZE / 2;
//             draw_line(x0, y0, x1, y1, bullet_color);
//             x0 = x1;
//             y0 = y1;
//         }
//     }

//     // --- 绘制子弹连线 2：连接所有已放置塔的位置 ---
//     bool first_tower_found = false;
//     int tx0 = 0, ty0 = 0;
//     for (int i = 0; i < 10; i++)
//     {
//         if (tower_placed[i])
//         {
//             if (!first_tower_found)
//             {
//                 tx0 = block_positions[i].x;
//                 ty0 = block_positions[i].y;
//                 first_tower_found = true;
//             }
//             else
//             {
//                 int tx1 = block_positions[i].x;
//                 int ty1 = block_positions[i].y;
//                 draw_line(tx0, ty0, tx1, ty1, bullet_color);
//                 tx0 = tx1;
//                 ty0 = ty1;
//             }
//         }
//     }
// }

int main(void) {
    // 1) 初始化 PS/2 键盘
    initPS2Keyboard();

    // 2) 获取显存控制器指针
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
    
    // 3) 显示 Intro 画面
    plot_image_intro(0, 0);

    // 4) 等待用户按下 Enter（扫描码 0x5A）
    char code;
    while (1) {
        code = readPS2ScanCode();
        if (code == 0x5A) {
            break;
        }
    }

    // 5) 初始化双缓冲：
    // 设置前缓冲区为 Buffer1 并调用 wait_for_vsync() 交换缓冲区，
    // 然后在前缓冲区绘制静态背景（包含棕色道路等）
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1[0];
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    plot_image_game(0, 0);  // 绘制静态背景

    // 设置后缓冲区为 Buffer2，并在后缓冲区预绘制静态背景
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2[0];
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    plot_image_game(0, 0);

    // 怪物初始位置及运动状态（根据你的棕色道路的起点调整）
    for (int i = 0; i < NUM_BOXES; i++)
    {
        boxes[i].x = 49;
        boxes[i].y = 180;
        boxes[i].dx = 0;
        boxes[i].dy = -1;
        boxes[i].color = colors[i];
    }

    // 6) 进入主循环：每帧先重绘背景，再绘制静态塔和动态怪物
    while (1) {
        // 将后缓冲区设为当前绘图目标
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        
        // --- 重绘静态背景 ---
        plot_image_game(0, 0);

        // 让新的怪物出现（类似每 60 帧出现一个盒子）
        frame_count++;
        if ((frame_count >= FRAMES_PER_BOX) && (num_draw_box < NUM_BOXES))
        {
            num_draw_box++;
            frame_count = 0;
        }

        // --- 绘制所有已放置的塔 ---
        for (int i = 0; i < 10; i++) {
            if (tower_placed[i]) {
                plot_image_tower1(block_positions[i].x, block_positions[i].y);
            }
        }

        // --- 动态元素：怪物运动 ---
        // 绘制已经出现的盒子
        for (int i = 0; i < num_draw_box && !monster_finished; i++)
        {
            update_monster_position(&boxes[i].x, &boxes[i].y, &boxes[i].dx, &boxes[i].dy);
            if (boxes[i].x > 260)
            { // 根据实际情况调整终点条件
                monster_finished = true;
            }
            else
            {
                plot_image_monster(boxes[i].x, boxes[i].y);
            }
        }
        
        
        // --- 处理键盘输入：塔的放置 ---
        // 使用非阻塞方式读取键盘扫描码，若没有新数据则返回 0
        code = readPS2ScanCode();
        if (code != 0) {
            if (code == 0xF0) {
                released = true;
            } else {
                if (!released) {
                    int block_idx = get_block_index_from_scan_code(code);
                    if (block_idx >= 0 && block_idx < 10) {
                        // 将对应塔标记为已放置，使其在后续帧中一直绘制
                        tower_placed[block_idx] = true;
                    }
                }
                released = false;
            }
        }

        for (int i = 0; i < 10; i++)
        {
            if (tower_placed[i])
            {
                // 从坐标原点 (0,0) 到塔的位置 (block_positions[i].x, block_positions[i].y)
                draw_line(120, 120, block_positions[i].x, block_positions[i].y, 0xFFFF);
            }
        }

        // --- 调用子弹函数：绘制子弹连线 ---
        //shoot_bullet();

        // --- 交换缓冲区 ---
        wait_for_vsync();
        // 交换后，后缓冲区完整画面将显示到屏幕上，下一帧继续在新后缓冲区绘制
    }

    return 0;
}*/