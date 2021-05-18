#ifndef __PLAY_H__
#define __PLAY_H__

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include "dlist.h"


#define MUSIC_PATH "/home/debian/music"

//共享内存
#define SHM_SIZE 4096  //大小
#define SHN_KEY 0x12345678 //key值

//播放模式
#define SEQUENCE      1         //顺序播放
#define SINGLE_LOOP   2         //单曲循环
#define LOOP          3         //循环播放

#define SONG_PATH_LENTH 256

//共享内存中存放的结构体
typedef struct _shm
{
    int play_mode;//播放模式
    int song_num;//歌曲总数
    Node* cur_song_p;//当前歌曲的节点指针
    pid_t ppid;//父进程id
    pid_t cpid;//子进程id
    pid_t gpid;//孙进程id
}SHM;

int get_music();//该函数失败时要释放双向链表
int init_shm();//初始化共享内存
int start_pause_continue();//开始播放 暂停 继续
int pause_play();//暂停播放
int continue_play();//继续播放
int next_song();//播放下一首
int prev_song();//播放上一首
int asr_function();//语音识别

#endif