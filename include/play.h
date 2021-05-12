#ifndef __PLAY_H__
#define __PLAY_H__

#define MUSIC_PATH "/home/debian/music"
#define SONGNAME_LENTH 64//歌曲名的最大长度

int get_music();//该函数失败时要释放双向链表
int init_shm();//初始化共享内存
int start_pause_continue();//开始播放 暂停 继续

#endif