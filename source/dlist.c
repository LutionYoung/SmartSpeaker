#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "dlist.h"

Node* head = NULL;

int dlist_create(Node** head,char* data)
{
    Node* p = (Node*)malloc(sizeof(Node));
    if(!p)
    {
        puts("create Node failed!");
        return -1;
    }
    p->prev = NULL;
    p->next = NULL;
    strcpy(p->data,data);

    *head = p;

    return 0;
}

int dlist_addtail(Node** head,char* data)
{
    Node* p =NULL;
    Node* q =NULL;
    Node* pNew = (Node*)malloc(sizeof(Node));
    if(!pNew)
    {
        puts("create Node failed!");
        return -1;
    }

    strcpy(pNew->data,data);
    pNew->next = NULL;

    p = *head;
    while(p)
    {
        q = p;
        p = p->next;
    }

    if(*head == NULL)
    {
        *head = pNew;
        pNew->prev = NULL;
    }
    else
    {
        pNew->prev = q;
        q->next = pNew;  
    }
    
    return 0;
}

int dlist_show_num(Node** head)
{
    int num = 0;
    Node* p = NULL;
    if(*head == NULL)
    {
        puts("dlist is empty");
        return -1;
    }

    p = *head;
    while(p)
    {
        //printf("song name:%s\n",p -> data);
        num++;
        p = p->next;
    }

    return num;
}

void dlist_free(Node** head)
{
    Node* p = *head;
    Node* q;
    if(!p)
    {
        puts("dlist is empty");
        return ;
    }

    while(p)
    {
        q = p;
        p = p->next;
        free(q);
    }
    *head = NULL;
    
    return ;
}