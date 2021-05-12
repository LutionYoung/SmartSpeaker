#include "main.h"
#include "device.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <time.h>
#include <pthread.h>

extern int g_ledRedfd;//led红灯
extern int g_ledGreenfd;//led绿灯
extern int g_ledBluefd;//led蓝灯
extern int g_playfd;//按一下播放暂停 按两下下一首 长按下一首 (语音助手暂时为长按更长时间)
int flag;//定时器的标志位   
/*
extern int g_volUpfd;//控制音量增加
extern int g_volDownfd;//控制音量减少
extern int g_voicefd;//语音识别按钮
*/

/*用于打开设备相关文件*/
int deviceInit()
{
    puts("Devices Init Start...");
    int ret = 0;
    ret += (g_ledBluefd = open(LED_BLUE_PATH,O_WRONLY));
    ret += (g_ledRedfd = open(LED_RED_PATH,O_WRONLY));
    ret += (g_ledGreenfd = open(LED_GREEN_PATH,O_WRONLY));
    if(ret < 0)
    {
        return FAILURE;
    }

    LedOn("blue");//初始化过程中亮蓝灯

    
    /*先判断gpio128 130 132有没有导出至用户空间*/
    /*
    if(access(VOL_UP_PATH,F_OK) < 0)
    {
        exportfd = open(GPIO_EXPORT_PATH,O_WRONLY);
        if(exportfd < 0)
        {
            perror("deviceInit:open");
            return FAILURE;
        }
        write(exportfd,"130",3);
        close(exportfd);
        usleep(200);//等待创建gpio设备文件
    }
    if(access(VOL_DOWN_PATH,F_OK) < 0)
    {
       exportfd = open(GPIO_EXPORT_PATH,O_WRONLY);
        if(exportfd < 0)
        {
            perror("deviceInit:open");
            return FAILURE;
        }
        write(exportfd,"128",3);
        close(exportfd);
        usleep(200);//等待创建gpio设备文件
    }
    if(access(VOICE_CTRL_PATH,F_OK) < 0)
    {
        exportfd = open(GPIO_EXPORT_PATH,O_WRONLY);
        if(exportfd < 0)
        {
            perror("deviceInit:open");
            return FAILURE;
        }
        write(exportfd,"132",3);
        close(exportfd);
        usleep(200);//等待创建gpio设备文件
    }
    usleep(900);//等待创建gpio设备文件

    ret += (g_volUpfd = open(VOL_UP_PATH,O_RDONLY));
    ret += (g_volDownfd = open(VOL_DOWN_PATH,O_RDONLY));
    ret += (g_voicefd = open(VOICE_CTRL_PATH,O_RDONLY));
    */
    ret += (g_playfd = open(KEY_PLAY_PATH,O_RDONLY));

    if(ret < 0)
    {
        return FAILURE;
    }

    puts("devices Init Done");
}

void redLedSwitch(const char* sw)
{
    write(g_ledRedfd,sw,strlen(sw));
}

void GreenLedSwitch(const char* sw)
{
    write(g_ledGreenfd,sw,strlen(sw));
}

void blueLedSwitch(const char* sw)
{
    write(g_ledBluefd,sw,strlen(sw));
}

void LedOn(const char* color)
{
    if(strcmp(color,"red") == 0)
    {
        redLedSwitch(LIGHT_ON);
        blueLedSwitch(LIGHT_OFF);
        GreenLedSwitch(LIGHT_OFF);
    }
    else if(strcmp(color,"blue") == 0)
    {
        redLedSwitch(LIGHT_OFF);
        blueLedSwitch(LIGHT_ON);
        GreenLedSwitch(LIGHT_OFF);
    }
    else if(strcmp(color,"green") == 0)
    {
        redLedSwitch(LIGHT_OFF);
        blueLedSwitch(LIGHT_OFF);
        GreenLedSwitch(LIGHT_ON);
    }
    else
    {
        printf("LedOn function param mistake\n");
    }
}

/*
struct input_event {
	struct timeval time;时间
	__u16 type;按键类型为EV_KEY
	__u16 code;读取的结构体仅对应一个按键，code可不必关心
	__s32 value;按下去为1，抬起为0
    }
*/

void* timer(void* arg)
{   
    
    struct timeval temp = {0,600000};
    select(0, NULL, NULL, NULL, &temp);
    
   /*
    usleep(600000);
    */
    flag = 0;
}

int key_parse()
{
    pthread_t tid = -1;
    flag = 1; 
    int count = 0;
    struct input_event ev = {0};
    fcntl(g_playfd,F_SETFL,FNDELAY);//此处将文件描述符设置为非阻塞方式，在while中让read函数一直读取g_playfd 
    if(pthread_create(&tid,NULL,timer,NULL) < 0)
    {
        perror("pthread_create");
        return -1;
    }
    
    while(flag)
    { 
        read(g_playfd,&ev,sizeof(ev));
        if(ev.type == EV_KEY && ev.value == 0)
        {
            count++;
        }
    }
    fcntl(g_playfd,F_SETFL,0);//将文件描述符恢复位非阻塞方式

    return count;
}