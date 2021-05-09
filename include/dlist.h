#ifndef __DLIST_H__
#define __DLIST_H__

typedef struct _node
{
    char data[64];
    struct _node* next;
    struct _node* prev;
}Node;

int dlist_create(Node** head,char* data);
int dlist_addtail(Node** head,char* data);
void dlist_show(Node** head);
void dlist_free(Node** head);

#endif