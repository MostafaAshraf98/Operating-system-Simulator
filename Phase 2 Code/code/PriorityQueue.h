#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Structs.h"
// priority Node


typedef struct PriorityQueue
{
   Node *head;
   int count;
} PriorityQueue;

PriorityQueue *newPriorityQueue()
{
   PriorityQueue *temp = (PriorityQueue *)malloc((sizeof(PriorityQueue)));
   (temp->head) = NULL;
   temp->count = 0;
   return temp;
}

int isEmpty(PriorityQueue **q)
{
   return (*q)->head == NULL;
}

process* front(PriorityQueue **q)
{
   if (isEmpty(q))
   {
      process* p=NULL;
      return p;
   }
   return ((*q)->head->p);
}

process* dequeue(PriorityQueue (**q))
{
   if (isEmpty(q))
   {
      process* p=NULL;
      return p;
   }
   Node *temp = ((*q)->head);
   ((*q)->head) = ((*q)->head)->next;
   //strcpy(temp->p->state,"running");
   process* returnedProcess = temp->p;
   free(temp);
   return returnedProcess;
}

void enqueue(PriorityQueue **q, process* p)
{
   if (isEmpty(q))
   {
      Node *temp = newNode(p);
      ((*q)->head) = temp;
      return;
   }
   Node *start = ((*q)->head);
   Node *temp = newNode(p);
   if (((*q)->head)->p->priority > p->priority)
   {
      temp->next = ((*q)->head);
      ((*q)->head) = temp;
   }
   else
   {
      while (start->next != NULL &&
             start->next->p->priority <= p->priority)
      {
         start = start->next;
      }
      temp->next = start->next;
      start->next = temp;
   }
}

void incrementWaintingTime(PriorityQueue **q)
{
   if (isEmpty(q))
      return;
   Node *start = ((*q)->head);
   start->p->WaitingTime++;
   while (start->next != NULL)
   {
      start->next->p->WaitingTime++;
      start = start->next;
   }
}

void printQueue(PriorityQueue **q)
{
   if (isEmpty(q))
      return;
   Node *start = ((*q)->head);
   printProcess(start->p);
   while (start->next != NULL)
   {
      printProcess(start->next->p);
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

//    PriorityQueue *pq = newPriorityQueue();
//    enqueue(&pq, p1);
//    enqueue(&pq, p2);
//    enqueue(&pq, p3);
//    printQueue(&pq);
//    incrementWaintingTime(&pq);
//    printQueue(&pq);
//    return 0;
// }