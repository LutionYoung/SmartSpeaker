#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include "device.h"
#include "socket.h"
#include "main.h"

extern int g_sockfd;
extern fd_set set_backup;//fd集合

void* handle(void* arg)//客户端与服务器的连接放在子线程中 避免影响主线程
{
    int count = 5;
    struct sockaddr_in server = {AF_INET};
    server.sin_port = htons(atoi(PORT));
    inet_pton(AF_INET,IP,&server.sin_addr);

    while(count--)
    {
        if(connect(g_sockfd,(struct sockaddr*)&server,sizeof(server)) < 0)
        {
            perror("connect:");
            sleep(2);//未连接上让这个线程睡眠2s后继续连接
        }
        else
        {
            LedOn("green");//连上服务器时亮绿灯
            puts("connect to server successfully!");
            FD_SET(g_sockfd,&set_backup);
            return NULL;
        }
    }
    /*未连接上服务器时红灯亮*/
    puts("Can not connect to server");
    LedOn("red");
    return NULL;
}

int sockInit()
{
    puts("Socket Init Start...");
    pthread_t tid;
    g_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(g_sockfd < 0)
    {
        perror("socket");
        return FAILURE;
    }

    if(pthread_create(&tid,NULL,handle,NULL) < 0)
    {
        printf("pthread_create failed\n");
        close(g_sockfd);
        return  FAILURE;
    }

}