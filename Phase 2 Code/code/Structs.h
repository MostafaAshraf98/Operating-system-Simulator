#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef struct process
{
   char state[20];
   int id;
   int arrivalTime;
   int priority;
   int runTime;
   int remainingTime;
   int WaitingTime;
   int memsize;
   int memStartAddr;
   int memEndAddr;
} process;

typedef struct node
{
   process* p;
   struct node *next;
} Node;

typedef struct memoryNode
{
   process* p;
   int start;
   int size;
   struct memoryNode *next;
} memoryNode;

void printProcess(process* p)
{
   printf("id is: %d\n", p->id);
   printf("prioriry is: %d\n", p->priority);
   // printf("state is: %s\n", p->state);
   printf("arrival times is: %d\n", p->arrivalTime);
   printf("runTime is: %d\n", p->runTime);
   printf("remaining time is: %d\n", p->remainingTime);
   printf("Waiting time is: %d\n\n", p->WaitingTime);
}

Node *newNode(process* p)
{
   Node *temp = (Node *)malloc(sizeof(Node));
   temp->p = p;
   temp->next = NULL;
   return temp;
}

memoryNode *newMemNode(process* p,int start,int size)
{
   memoryNode *temp = (memoryNode *)malloc(sizeof(memoryNode));
   temp->p = p;
   temp->start=start;
   temp->size=size;
   temp->next = NULL;
   return temp;
}