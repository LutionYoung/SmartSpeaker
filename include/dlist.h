#ifndef __DLIST_H__
#define __DLIST_H__
#define SONGNAME_LENTH 64//歌曲名的最大长度

typedef struct _node
{
    char data[SONGNAME_LENTH];
    struct _node* next;
    struct _node* prev;
}Node;

int dlist_create(Node** head,char* data);
int dlist_addtail(Node** head,char* data);
int dlist_show_num(Node** head);
void dlist_free(Node** head);

#endif