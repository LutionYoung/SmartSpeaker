#include "play.h"
#include "asrmain.h"
#include "cJSON.h"
#include "play.h"

extern Node* head;
extern char* result_backup;//语音识别时返回识别的数据起始地址备份

void* g_shm_addr;
int play_flag;//-1为开机后未播放过歌曲或播放已结束 1为歌曲播放中 0为播放已暂停
int ctl_flag;//0为自动播放 -1为上一首 1为下一首 2为语音指令导致的歌曲切换
Node* voice_song;//语音点歌查找到的歌曲节点

int voice_play(char* name)
{
    Node* cur_song = NULL;
    char search_name[64] = {0};
    strncpy(search_name,name,strlen(name)-3);
    //printf("voice_play1:%s\n",search_name);
    cur_song = dlist_search(&head,search_name);
    if(cur_song != NULL)
    {
        printf("找到歌曲:%s\n",cur_song->data);
        //执行播放匹配到的歌曲
        if(play_flag != -1)//当前已播放歌曲 或播放暂停(子进程已创建)
        {
            SHM shm = {0};
            memcpy(&shm,g_shm_addr,sizeof(shm));
            ctl_flag = 2;//2表示为语音指令导致的歌曲切换
            play_flag = 1;
            shm.cur_song_p = cur_song;
            memcpy(g_shm_addr,&shm,sizeof(shm));
            kill(shm.gpid,SIGKILL);//杀死孙进程
            return 0;
        }
        else//当前还未播放过歌曲(子进程未创建)
        {
            ctl_flag = 2;//2表示为语音指令导致的歌曲切换
            voice_song = cur_song;
            play_music();
            return 0;
        }
    }
    else
    {
        puts("未匹配到歌曲");
    }

    return 0;
}

void get_cur_song_path(SHM* shm,char* path)
{
    strcpy(path,MUSIC_PATH);
    strcat(path,"/");
    strcat(path,shm->cur_song_p->data);
}

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

/*信号处理函数*/
void handler(int sig)
{
    if(sig == SIGUSR1)//孙进程歌曲播放结束，子进程发信号给父进程，让父进程更新prev/current/next song
    {
        SHM shm;
        memcpy(&shm,g_shm_addr,sizeof(shm));

        /*人为按键造成上一首、下一首歌的切换*/
        if(ctl_flag == 1)//由按键引起的下一首的播放
        { 
            if(shm.cur_song_p->next == NULL)
            {
                shm.cur_song_p = head;
            }
            else
            {
                shm.cur_song_p = shm.cur_song_p->next;
            }
            
            memcpy(g_shm_addr,&shm,sizeof(shm));
            ctl_flag = 0;//将ctl_flag拉回0 后边自动播放  
            return ;
        }
        else if(ctl_flag == -1)//由按键引起的上一首的播放
        {
            if(shm.cur_song_p->prev == NULL)
            {
                shm.cur_song_p = head;
            }
            else
            {
                shm.cur_song_p = shm.cur_song_p->prev;
            }
            
            memcpy(g_shm_addr,&shm,sizeof(shm));
            ctl_flag = 0;//将ctl_flag拉回0 后边自动播放    
            return ;
        }
        else//由语音指令指定播放歌曲而导致的歌曲切换 ctl_flag值为2时
        {
            ctl_flag = 0;//将ctl_flag拉回0 后边自动播放
            return ;
        }

        /*自动播放造成下一首歌的切换*/
        if(shm.play_mode == SEQUENCE)//顺序播放时
        {
            shm.cur_song_p = shm.cur_song_p->next;//下一首歌的节点(可能为NULL)
            if(shm.cur_song_p == NULL)
            {
                play_flag = -1;//播放已结束
            }else
            {
                printf("next song name:%s\n",shm.cur_song_p->data);
            }
        }
        else if(shm.play_mode == LOOP)//循环播放时
        {
            if(shm.cur_song_p->next == NULL)//播放完最后一首歌时 拉回到头节点
            {
                shm.cur_song_p = head;
            }
            else
            {
                shm.cur_song_p = shm.cur_song_p->next;//不是最后一首歌时 播放下一首
            }
        }
        else//单曲循环时
        {
            //不修改current song
        }
        memcpy(g_shm_addr,&shm,sizeof(shm));  
    }
}

int play_music()
{
    /*共享内存操作*/
    SHM shm = {SEQUENCE};
    shm.song_num = dlist_show_num(&head);
    signal(SIGUSR1,handler);
    if(shm.song_num < 0)
    {
        puts("no song to play");
        return -1;
    }

    if(ctl_flag != 2)//当不因为语音点歌导致的播放歌曲时 指定头节点为要播放的歌曲
    {
        shm.cur_song_p = head;
    }
    else//如果因为语音点歌指定了歌曲 则什么也不做 已在...函数中指定好当前要播放的歌曲
    {
        ctl_flag = 0;//拉回ctl_flag的值为0 即后边自动播放
        shm.cur_song_p = voice_song;//写入语音点歌匹配到的歌曲节点指针
    }

    shm.ppid = getpid();
    

    /*创建子进程、孙进程*/
    pid_t cpid = fork();
    if(cpid < 0)
    {
        perror("fork");
        return -1;
    }
    if(cpid > 0)//父进程
    {
        shm.cpid = cpid;
        memcpy(g_shm_addr,&shm,sizeof(shm));
        play_flag = 1;//歌曲正在播放
        return 0;
    }
    else//子进程
    {
        while(1)
        {
            init_shm();//子进程连接共享内存并映射到g_shm_addr上
            bzero(&shm,sizeof(shm));
            memcpy(&shm,g_shm_addr,sizeof(shm));//可获取当前要播放的、下一首、上一首歌曲名
            if(shm.cur_song_p == NULL)//当前歌曲节点为空时停止播放
            {
                puts("Node is NULL,finishing playing");
                shmdt(g_shm_addr);
                exit(0);
            }
            char path[SONG_PATH_LENTH] = {0};
            get_cur_song_path(&shm,path);//得到当前要播放的歌曲的路径,以便让孙进程使用
            pid_t gpid = fork();
            if(gpid < 0)
            {
                perror("fork");
                exit(-1);
            }
            if(gpid > 0)//子进程
            {
                shm.gpid = gpid;
                memcpy(g_shm_addr,&shm,sizeof(shm));
                shmdt(g_shm_addr);//断开共享内存
                wait(NULL);
                kill(shm.ppid,SIGUSR1);//发信号给父进程让父进程更新当前歌曲
                puts("finish playing and wait for father process updating current song");
                usleep(5000);//暂时让子进程等待父进程5ms更新cur_song_p 后期采用信号量解决同步问题
            }
            else//孙进程
            {
                printf("paly song path:%s\n",path);
                execl("/usr/bin/madplay","madplay",path,NULL);
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
        pause_play();//暂停播放(通过发信号)
    }
    else//当前播放已暂停
    {
        continue_play();//继续播放(通过发信号)
    }
}


int pause_play()//暂停播放
{
    SHM shm = {0};
    memcpy(&shm,g_shm_addr,sizeof(shm));
    kill(shm.gpid,SIGSTOP);//暂停孙进程(暂停音乐播放)
    play_flag = 0;
}

int continue_play()//继续播放
{
    SHM shm = {0};
    memcpy(&shm,g_shm_addr,sizeof(shm));
    kill(shm.gpid,SIGCONT);//继续运行孙进程(继续音乐播放)
    play_flag = 1;
}

int next_song()
{
    if(play_flag == -1)
    {
        play_music();
        return 0;
    }
    SHM shm = {0};
    memcpy(&shm,g_shm_addr,sizeof(shm));
    ctl_flag = 1;
    play_flag = 1;
    kill(shm.gpid,SIGKILL);//杀死孙进程
    return 0;
}

int prev_song()
{
    if(play_flag == -1)
    {
        play_music();
        return 0;
    }
    SHM shm = {0};
    memcpy(&shm,g_shm_addr,sizeof(shm));
    ctl_flag = -1;
    play_flag = 1;
    kill(shm.gpid,SIGKILL);//杀死孙进程
    return 0;
}

int asr_function()
{
    int asr_ctl_flag = 0;
    if(play_flag == 1)//如果当前正在播放音乐
    {
        pause_play();//暂停播放(通过发信号)
        asr_ctl_flag = 1;
    }

    pid_t pid;
    pid = fork();
    if(pid < 0)
    {
        perror("asr_function fork failed");
        return -1;
    }
    if(pid == 0)
    {
        execl("/usr/bin/arecord","arecord","-d","5","-f","cd","-r","16000","-c","1","-t","wav","/tmp/asr.wav",NULL);
        //exit(0);
    }
    else
    {
        waitpid(pid,NULL,0);
        result_backup = NULL;
        asrmain();
        if(result_backup == NULL)
        {
            puts("result_backup asr failed");
            return -1;
        }
        //printf("\nasr result:%s\n",result_backup);
        

        /*解析接收到的json数据*/
        cJSON* json = cJSON_Parse(result_backup);//加载json原始数据
        cJSON* jsonVal = cJSON_GetObjectItem(json,"result");//获取"result"关键字所对应的value 该value是个字符串数组
        cJSON* jsonObj = cJSON_GetArrayItem(jsonVal,0);

        /*对返回的命令解析后选择处理方式*/
        printf("json parse:%s\n",jsonObj->valuestring);

        char buf[128] = {0};
        char* ret = NULL;
        strncpy(buf,jsonObj->valuestring,127);

        if((ret = strstr(buf,"播放音乐")) != NULL)
        {
            puts("语音识别:播放音乐");
            if(play_flag == -1)
            {
                if(play_music() < 0)//第一次播放音乐
                {
                    exit (-1);
                }
            }
            else if(play_flag == 1)
            {
                //正在播放音乐时 什么都不做
            }
            else
            {
                continue_play();//继续播放(通过发信号)
            }
        }
        else if((ret = strstr(buf,"停止播放")) != NULL)
        {
            puts("语音识别:停止播放");
            if(play_flag == 1)
            {
                pause_play();//暂停播放(通过发信号)
            }
        }
        else if((ret = strstr(buf,"上一首")) != NULL)
        {
            puts("语音识别:上一首");
            prev_song();
        }
        else if((ret = strstr(buf,"下一首")) != NULL)
        {
            puts("语音识别:下一首");
            next_song();
        }
        else if((ret = strstr(buf,"继续播放")) != NULL)
        {
            puts("语音识别:继续播放");
            if(play_flag == -1)
            {
                if(play_music() < 0)//第一次播放音乐
                {
                  exit (-1);
                }
                else//当前播放已暂停
                {
                    continue_play();//继续播放(通过发信号)
                }
            }
        }
        else if(strstr(buf,"播放歌曲") != NULL && (*(buf+12) != '\0'))
        {
                printf("语音识别歌曲名:%s\n",buf+12);
                voice_play(buf+12);
        }
        else
        {
            puts("未成功识别语音指令");
            if(asr_ctl_flag == 1)
            {
                continue_play();//若以上结果均没有则继续播放歌曲
            }
        }
        
        /*释放资源*/
        if(json != NULL)
        {
            cJSON_Delete(json);
        }
        if(result_backup != NULL)
        {
             free(result_backup); 
        }
        
        return 0;
    }
}