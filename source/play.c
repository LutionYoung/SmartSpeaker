#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include "play.h"
#include "dlist.h"

extern Node* head;

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
    //dlist_show(&head);
}
