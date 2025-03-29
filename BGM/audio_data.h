#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

// 声明音频数据数组（`extern` 只声明，不定义）
extern const unsigned char music[];
extern const unsigned int MUSIC_LENGTH; // 用 `unsigned int` 避免类型冲突

// 声明音频播放函数
void play_audio_continuous(void);
void audio_init(void);

#endif // AUDIO_H
