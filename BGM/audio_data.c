#include <stdint.h>
#include "audio_data.h" // 如果你的音乐数据在这里定义，就保留此行

//=====================  寄存器定义  =====================//
#define AUDIO_CONTROL (*(volatile uint32_t *)0xFF203040)
#define AUDIO_FIFO_SPACE (*(volatile uint32_t *)0xFF203044)
#define AUDIO_LEFT_DATA (*(volatile uint32_t *)0xFF203048)
#define AUDIO_RIGHT_DATA (*(volatile uint32_t *)0xFF20304C)

//=====================  宏定义  =========================//
// 用于清空 FIFO 的位
#define AUDIO_CLEAR_READ_FIFO (1 << 2)
#define AUDIO_CLEAR_WRITE_FIFO (1 << 3)

//=====================  函数声明  =======================//
void audio_init(void);
void play_audio_continuous(void);

//=====================  函数实现  =======================//
/*
 * 函数: audio_init
 * 描述: 初始化音频系统，清空 FIFO
 */
void audio_init(void)
{
    // 设置清空 FIFO
    AUDIO_CONTROL |= (AUDIO_CLEAR_READ_FIFO | AUDIO_CLEAR_WRITE_FIFO);
    // 等待清除操作完成后清除位
    AUDIO_CONTROL &= ~(AUDIO_CLEAR_READ_FIFO | AUDIO_CLEAR_WRITE_FIFO);
}

/*
 * 函数: play_audio_continuous
 * 描述: 轮询音频端口输出 FIFO，当 FIFO 中有空位时，将一个采样数据写入左右声道，
 *       当播放到末尾时自动循环，从而持续播放音乐。
 */
void play_audio_continuous(void)
{
    int index = 0; // 播放索引

    while (1)
    {
        // 读取 FIFO 空余空间寄存器
        uint32_t fifo_space = AUDIO_FIFO_SPACE;
        uint8_t ws_left = fifo_space & 0xFF;         // 左声道 FIFO 空位
        uint8_t ws_right = (fifo_space >> 8) & 0xFF; // 右声道 FIFO 空位

        // 如果左右写 FIFO 都有空位，则写入一个采样数据
        if (ws_left > 0 && ws_right > 0)
        {
            int16_t sample = music[index];       // 读取 PCM 采样数据
            AUDIO_LEFT_DATA = (uint32_t)sample;  // 写入左声道 FIFO
            AUDIO_RIGHT_DATA = (uint32_t)sample; // 写入右声道 FIFO

            // 移动到下一个采样数据，循环播放
            index++;
            if (index >= MUSIC_LENGTH)
                index = 0;
        }
    }
}

//=====================  主函数  =========================//
int main(void)
{
    audio_init();            // 初始化音频系统
    play_audio_continuous(); // 循环播放音乐
    return 0;
}
