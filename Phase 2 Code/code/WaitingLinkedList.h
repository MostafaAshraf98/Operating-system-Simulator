#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Structs.h"

//Linked list shall be used in: waiting list for all algorithms

typedef struct WaitingLinkedList
{
    Node *head;
    int count;

} WaitingLinkedList;

WaitingLinkedList *newWaitingLinkedList()
{
    WaitingLinkedList *temp = (WaitingLinkedList *)malloc((sizeof(WaitingLinkedList)));
    (temp->head) = NULL;
    temp->count = 0;
    return temp;
}

int isEmptyWait(WaitingLinkedList **q)
{
    return (*q)->head == NULL;
}

process *frontWait(WaitingLinkedList **q)
{
    if (isEmptyWait(q))
    {
        process *p = NULL;
        return p;
    }
    return ((*q)->head->p);
}

process *pop(WaitingLinkedList(**q))
{
    if (isEmptyWait(q))
    {
        process *p = NULL;
        return p;
    }
    Node *temp = ((*q)->head);
    ((*q)->head) = ((*q)->head)->next;
    process *returnedProcess = temp->p;
    free(temp);
    return returnedProcess;
}

void push(WaitingLinkedList **q, process *p)
{
    if (isEmptyWait(q))
    {
        Node *temp = newNode(p);
        ((*q)->head) = temp;
        return;
    }
    Node *start = ((*q)->head);
    Node *temp = newNode(p);
    while (start->next != NULL)
    {
        start = start->next;
    }
    temp->next = start->next;
    start->next = temp;
}

void printLinkedList(WaitingLinkedList **q)
{
    if (isEmptyWait(q))
        return;
    Node *start = ((*q)->head);
    printProcess(start->p);
    while (start->next != NULL)
    {
        printProcess(start->next->p);
        start = start->next;
    }
}

void incrementWaintingTimeWait(WaitingLinkedList **q)
{
   if (isEmptyWait(q))
      return;
   Node *start = ((*q)->head);
   start->p->WaitingTime++;
   while (start->next != NULL)
   {
      start->next->p->WaitingTime++;
      start = start->next;
   }
}

// int main()
// {
//    process* p1;
//    p1=(process*)malloc(sizeof(process));
//    strcpy(p1->state,"running");
//    p1->id = 1;
//    p1->priority = 1;
//    p1->arrivalTime = 1;
//    p1->remainingTime = 3;
//    p1->runTime = 4;
//    p1->WaitingTime = 6;

//    process* p2;
//    p2=(process*)malloc(sizeof(process));
//    strcpy(p1->state,"ready");
//    p2->priority = 2;
//    p2->id = 2;
//    p2->arrivalTime = 10;
//    p2->remainingTime = 32;
//    p2->runTime = 14;
//    p2->WaitingTime = 64;

//    process* p3;
//    p3=(process*)malloc(sizeof(process));
//    strcpy(p1->state,"running");
//    p3->priority = 0;
//    p3->id = 3;
//    p3->arrivalTime = 11;
//    p3->remainingTime = 31;
//    p3->runTime = 12;
//    p3->WaitingTime = 64;

//    WaitingLinkedList *pq = newWaitingLinkedList();
//    push(&pq, p1);
//    push(&pq, p2);
//    push(&pq, p3);
//    printLinkedList(&pq);
//    return 0;
// }
