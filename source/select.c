#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include "select.h"
#include "main.h"
#include "device.h"
#include "play.h"
#include "asrmain.h"

extern int g_ledRedfd;//led红灯
extern int g_ledBluefd;//led蓝灯
extern int g_ledGreenfd;//led绿灯
extern int g_playfd;//按一下播放暂停 按两下下一首 长按上一首
/*
extern int g_volUpfd;//控制音量增加
extern int g_volDownfd;//控制音量减少
extern int g_voicefd;//语音识别按钮
*/
extern int g_sockfd;//socket
extern void* g_shm_addr;
extern int play_flag;
fd_set set_backup;//fd集合

int myselect()
{
    fd_set set;
    int ret = 0;
    int max_fd = (g_sockfd>g_playfd?g_sockfd:g_sockfd);
    FD_ZERO(&set);


    /*暂时先监听socket和play按钮按下情况*/
    FD_SET(g_playfd,&set_backup);

    struct input_event ev = {0};
    //char buf[64] = {0};
    struct timeval time = {1,0};
    while(1)
    {
        set = set_backup;
        ret = select(max_fd+1,&set,NULL,NULL,&time);
        if(ret < 0)
        {
            perror("select:");
        }
        if(ret == 0)
        {
            continue;
        }
        if(ret > 0)
        {
            if(FD_ISSET(g_sockfd,&set) > 0)
            {  
                //服务器端发来消息
                /*
                bzero(buf,sizeof(buf));
                recv(g_sockfd,buf,sizeof(buf)-1,0);
                printf("server msg:%s\n",buf);
                */
            }
            if(FD_ISSET(g_playfd,&set) > 0)
            {
                //play按键有输入
                
                read(g_playfd,&ev,sizeof(ev));
                if(ev.type == EV_KEY && ev.value ==1)
                {   
                    switch (key_parse())
                    {
                        case -1:
                            return -1;
                        case 0://长按语音识别
                        {
                            //可以加入合成的音频提醒用户说话
                            if(asr_function() < 0)
                            {
                                puts("asr failed!");
                            }

                            /*测试*/
                            /*
                            puts("set single loop");
                            SHM shm;
                            memcpy(&shm,g_shm_addr,sizeof(shm));
                            shm.play_mode = SINGLE_LOOP;
                            memcpy(g_shm_addr,&shm,sizeof(shm));
                            */
                            
                            break;
                        }
                        case 1://按一下 开始播放/播放/暂停
                            puts("button:play/stop");
                            start_pause_continue();
                            break;
                        case 2://连按两下 下一首
                        {
                            puts("button:next song");   //若开机还没有播放过歌曲 下一首功能为播放
                            next_song();

                            /*测试*/
                            /*
                            puts("set loop");
                            SHM shm;
                            memcpy(&shm,g_shm_addr,sizeof(shm));
                            shm.play_mode = LOOP;
                            memcpy(g_shm_addr,&shm,sizeof(shm));
                            */

                            break;
                        }
                        case 3://连按三下
                        {
                            puts("button:prev song");       //若开机还没有播放过歌曲 上一首功能为播放
                            prev_song();

                            /*测试*/
                            /*
                            puts("set sequenc");
                            SHM shm;
                            memcpy(&shm,g_shm_addr,sizeof(shm));
                            shm.play_mode = SEQUENCE;
                            memcpy(g_shm_addr,&shm,sizeof(shm));
                            */

                            break;
                        }
                        default:
                            puts("button:invalid");
                            break;
                    }
                }
            }
        }  
    }
}