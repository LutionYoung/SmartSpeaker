#ifndef __DEVICE_H__
#define __DEVICE_H__

#define LED_RED_PATH        "/sys/class/leds/red/brightness"
#define LED_GREEN_PATH      "/sys/class/leds/green/brightness"
#define LED_BLUE_PATH      "/sys/class/leds/blue/brightness"
#define KEY_PLAY_PATH       "/dev/input/event1"                    //暂时通过输入子系统来判断按键是否按下
/*
#define VOL_UP_PATH         "/sys/class/gpio/gpio130/value"        //GPIO5_2
#define VOL_DOWN_PATH       "/sys/class/gpio/gpio128/value"        //GPIO5_0
#define VOICE_CTRL_PATH     "/sys/class/gpio/gpio132/value"        //GPIO5_4
#define GPIO_EXPORT_PATH    "/sys/class/gpio/export" 
              //用于导出gpio接口到用户空间
*/

int deviceInit();
void redLedSwitch(const char* sw);
void GreenLedSwitch(const char* sw);
void blueLedSwitch(const char* sw);

void LedOn(const char* color);



#endif