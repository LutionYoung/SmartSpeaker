#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"
#include "device.h"
#include "socket.h"
#include "select.h"
#include "play.h"

int g_ledRedfd;//led红灯
int g_ledGreenfd;//led绿灯
int g_ledBluefd;//led蓝灯
int g_playfd;//按一下播放暂停 按两下下一首 长按上一首
/*
int g_volUpfd;//控制音量增加
int g_volDownfd;//控制音量减少
int g_voicefd;//语音识别按钮
*/
int g_sockfd;


int main(void)
{   
    if(deviceInit() < 0)
    {
        printf("device init failed\n");
        exit(-1);
    }

    if(sockInit() < 0) //socket初始化失败亮红灯 服务器未连接上亮红灯
    {
        printf("socket init failed\n");
        redLedSwitch(LIGHT_ON);
    }
    
    
    if(get_music() < 0)
    {
        close(g_sockfd);
        close(g_ledBluefd);
        close(g_ledGreenfd);
        close(g_ledRedfd);
        close(g_playfd);
        exit(-1);
    }
    
    
    if(init_shm() < 0)
    {
        return -1;
    }

    if(myselect() < 0)
    {
        close(g_sockfd);
        close(g_ledBluefd);
        close(g_ledGreenfd);
        close(g_ledRedfd);
        close(g_playfd);
        exit(-1);
    }

    while(1)
    ;

}


