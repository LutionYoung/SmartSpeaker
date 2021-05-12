#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include "dlist.h"
#include "play.h"
#include "dlist.h"

//共享内存
#define SHM_SIZE 4096  //大小
#define SHN_KEY 0x12345678 //key值

//播放模式
#define SEQUENCE 1 //顺序播放
#define RANDOM   2 //随机播放
#define LOOP     3 //循环播放  

//共享内存中存放的结构体
typedef struct _shm
{
    int play_mode;//播放模式
    int song_num;//歌曲总数
    char cur_song[SONGNAME_LENTH];//当前播放的歌曲
    char next_song[SONGNAME_LENTH];//下一首歌曲
    char prev_song[SONGNAME_LENTH];//上一首歌曲
    pid_t ppid;//父进程id
    pid_t cpid;//子进程id
    pid_t gpid;//孙进程id
}SHM;

extern Node* head;
void* g_shm_addr;
int play_flag;//-1为开机后未播放过歌曲或顺序播放完最后一首音乐 1为歌曲播放中 0为播放已暂停

int endof_mp3(char* name)
{
    char filename[256+1] = {0};
    strcpy(filename,name);
    char* p = strstr(filename,".mp3");
    if(!p)
    {
        return -1;
    }

    if(strlen(p) == 4)
    {
        return 0;
    }
    
    return -1;
}

int get_music()
{
    head = NULL;
    DIR* dir = opendir(MUSIC_PATH);
    if(!dir)
    {
        perror("opendir");
        return -1;
    }

    struct dirent* ent = NULL;
    while(ent = readdir(dir))
    {
        if(ent->d_type != DT_REG)
        {
            continue;
        }

        if(endof_mp3(ent->d_name) < 0)
        {
            continue;
        }
        if(dlist_addtail(&head,ent->d_name) < 0)
        {
            dlist_free(&head);
            return -1;
        }
    }
    play_flag = -1;//开机后还未播放音乐
    //dlist_show(&head);

    return 0;
}

int init_shm()
{
    int shm_fd = shmget(SHN_KEY,SHM_SIZE,IPC_CREAT|0666);//父进程中创建共享内存
    if(shm_fd < 0)
    {
        perror("shmget");
        return -1;
    }

    g_shm_addr = shmat(shm_fd,NULL,0);//映射共享内存
    if(g_shm_addr == (void*)-1)
    {
        perror("shmat");
        return -1;
    }

    return 0;
}

int play_music()
{
    /*共享内存操作*/
    SHM shm = {SEQUENCE};
    shm.song_num = dlist_show_num(&head);
    if(shm.song_num < 0)
    {
        puts("no song to play");
        return -1;
    }
    else if(shm.song_num == 1)//当前只有一首歌时
    {
        strcpy(shm.cur_song,head->data);
        strcpy(shm.next_song,head->data);
        strcpy(shm.prev_song,head->data);
    }
    else//当前有两首歌及以上时
    {
        strcpy(shm.cur_song,head->data);
        strcpy(shm.next_song,head->next->data);
        strcpy(shm.prev_song,"NONESONG");
    }
    shm.ppid = getpid();
    memcpy(g_shm_addr,&shm,sizeof(shm));

    /*创建子进程、孙进程*/
    pid_t cpid = fork();
    if(cpid < 0)
    {
        perror("fork");
        return -1;
    }
    if(cpid > 0)//父进程
    {
        return 0;
    }
    else//子进程
    {
        init_shm();//子进程连接共享内存并映射到g_shm_addr上
        shm.cpid = getpid();
        while(1)
        {
            memcpy(&shm,g_shm_addr,sizeof(shm));//可获取当前要播放的、下一首、上一首歌曲名
            ...const char* song_path = get_cur_song_path(shm.cur_song);//得到当前要播放的歌曲的路径,以便让孙进程使用
            pid_t gpid = fork()
            if(gpid < 0)
            {
                perror("fork");
                exit(-1);
            }
            if(gpid > 0)//子进程 (在暂停、播放的信号处理函数中要断开共享内存)
            {
                shm.gpid = gpid;
                memcpy(g_shm_addr,&shm,sizeof(shm));
                wait(NULL);
            }
            else//孙进程
            {

            }
        }
        /*调试信息*/
        /*
        puts("child process running...");
        puts("start playing");
        printf("current song:%s\n",shm.cur_song);
        printf("next song:%s\n",shm.next_song);
        printf("prev song:%s\n",shm.prev_song);
        */
    }
}

int start_pause_continue()//对应按键按以下的情形
{
    if(play_flag == -1)
    {
        if(play_music() < 0)//第一次播放音乐
        {
           exit (-1);
        }
    }
    else if(play_flag == 1)//当前正在播放音乐
    {
        //pause_play();//暂停播放(通过发信号)
    }
    else//当前播放已暂停
    {
        //continue_play();//继续播放(通过发信号)
    }
}