//#pragma once
// #include "headers.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

typedef short bool;
#define true 1
#define false 0

#include "Structs.h"

typedef struct MemLinkedList
{
    memoryNode *head;
    memoryNode *lastAllocated;
    int count;

} MemLinkedList;

void printNode(memoryNode *node)
{
    printf("start: %d\n", node->start);
    printf("size: %d\n", node->size);
    if (node->p == NULL)
        return;
    printProcess(node->p);
}

MemLinkedList *newMemLinkedList()
{
    MemLinkedList *temp = (MemLinkedList *)malloc((sizeof(MemLinkedList)));
    memoryNode *node = newMemNode(NULL, 0, 1024);
    (temp->head) = node;
    temp->lastAllocated = node;
    temp->count = 0;
    return temp;
}

// int isEmpty(MemLinkedList **q)
// {
//     return (*q)->head == NULL;
// }

// process *front(MemLinkedList **q)
// {
//     if (isEmpty(q))
//     {
//         process *p = NULL;
//         return p;
//     }
//     return ((*q)->head->p);
// }

void freeMem(MemLinkedList(**q), process* p)
{
    if ( (*q) == NULL)
    {
        memoryNode *m = NULL;
        return;
    }
    memoryNode* startNode = (*q)->head;
    memoryNode* previousNode = NULL;
    while ( startNode != NULL )
    {
        if ( startNode->p->id == p->id )
        {
            startNode->p = NULL;

            if ( previousNode != NULL && previousNode->p == NULL)
            {  
                int newSize = previousNode->size + startNode->size;
                previousNode->next = startNode->next;
                previousNode->size = newSize;
                free(startNode);
            }
            if ( startNode->next != NULL && startNode->next->p == NULL )
            {
                int newSize = startNode->size + startNode->next->size;
                memoryNode* temp = startNode->next;
                startNode->next = temp->next;
                startNode->size = newSize;
                free(temp);
            }

            return;
        }

        previousNode = startNode;
        startNode = startNode->next;
    }
}

bool pushMem(MemLinkedList **q, process *p, int algorithm)
{
    int size = p->memsize;
    memoryNode *startNode = ((*q)->head);
    if (startNode->next == NULL) // in case the memory is empty
    {
        memoryNode *temp = newMemNode(p, 0, size);
        temp->next = startNode;
        startNode->start = size;
        startNode->size = startNode->size - size;
        (*q)->lastAllocated = temp;
        (*q)->head = temp;
        return true;
    }

    memoryNode *chosenNode = NULL;
    memoryNode *previousNode = NULL;
    memoryNode* prevs = NULL;

    switch (algorithm)
    {
    case 1:                       // first fit
        while (startNode != NULL) // i am holding a node called startNode
        {
            if (startNode->p == NULL && startNode->size >= size) // if start node is not allocated and its size is greater than the required size
            {
                chosenNode = startNode;
                break;
            }
            previousNode = startNode;
            startNode = startNode->next;
        }

        if (chosenNode == NULL)
            return false;
        // now i want to insert a new node with the size of the process that is passed and its start is the start of the node that i found
        // and the node itself will reduce its size by the value of the size
        // must handle if the size is exactly equal
        if (chosenNode->size == size) // if their sizes exactly matches
        {
            chosenNode->p = p;
            return true;
        }
        else // if the size of the chosenNode is greater than the size of the process
        {

            memoryNode *temp = newMemNode(p, chosenNode->start, size); // create a new Node
            temp->next = chosenNode;
            if (previousNode == NULL) // this means that the chosen node is the head
                (*q)->head = temp;
            else
                previousNode->next = temp;
            chosenNode->size = chosenNode->size - size;

            chosenNode->start = chosenNode->start + size;
            return true;
        }
        break;
    case 2: // next fit
        startNode = (*q)->lastAllocated;
        if (startNode->next == NULL) // if the last allocated is the last element in the linked list
            startNode = (*q)->head;
        memoryNode *theStartingNextfitNode = startNode;
        do // i am holding a node called startNode
        {
            if (startNode->p == NULL && startNode->size >= size) // if start node is not allocated and its size is greater than the required size
            {
                chosenNode = startNode;
                break;
            }
            previousNode = startNode;
            startNode = startNode->next;
            if (startNode == NULL)
                startNode = (*q)->head;
        } while (startNode != theStartingNextfitNode);
        if (chosenNode == NULL)
            return false;

        // now i want to insert a new node with the size of the process that is passed and its start is the start of the node that i found
        // and the node itself will reduce its size by the value of the size
        // must handle if the size is exactly equal
        if (chosenNode->size == size) // if their sizes exactly matches
        {
            chosenNode->p = p;
            (*q)->lastAllocated = chosenNode;
            return true;
        }
        else // if the size of the chosenNode is greater than the size of the process
        {
            memoryNode *temp = newMemNode(p, chosenNode->start, size); // create a new Node
            temp->next = chosenNode;
            if (previousNode == NULL) // this means that the chosen node is the head
                (*q)->head = temp;
            else
                previousNode->next = temp;
            chosenNode->size = chosenNode->size - size;
            chosenNode->start = chosenNode->start + size;
            (*q)->lastAllocated = temp;
            return true;
        }
        break;
    case 3:                       // case best fit ( the smallest)
        while (startNode != NULL) // i am holding a node called startNode
        {
            if (startNode->p == NULL && startNode->size >= size) // if start node is not allocated and its size is greater than the required size
            {
                if (chosenNode == NULL || chosenNode->size > startNode->size)
                {
                    chosenNode = startNode;
                }
            }
            if ( chosenNode == startNode )
            {
                previousNode = prevs;
            }
            prevs = startNode;
            startNode = startNode->next;
        }
        if (chosenNode == NULL)
            return false;

        // now i want to insert a new node with the size of the process that is passed and its start is the start of the node that i found
        // and the node itself will reduce its size by the value of the size
        // must handle if the size is exactly equal
        if (chosenNode->size == size) // if their sizes exactly matches
        {
            chosenNode->p = p;
            return true;
        }
        else // if the size of the chosenNode is greater than the size of the process
        {
            memoryNode *temp = newMemNode(p, chosenNode->start, size); // create a new Node
            temp->next = chosenNode;
            if (previousNode == NULL) // this means that the chosen node is the head
                (*q)->head = temp;
            else
                previousNode->next = temp;
            chosenNode->size = chosenNode->size - size;
            chosenNode->start = chosenNode->start + size;
            return true;
        }
        break;
    default:
        break;
    }
}

void printMemLinkedList(MemLinkedList **q)
{
    memoryNode *startNode = ((*q)->head);
    printNode(startNode);
    while (startNode->next != NULL)
    {
        printNode(startNode->next);
        startNode = startNode->next;
    }
}

// void assignMemNode(memoryNode *N1, memoryNode *N2)
// {
//     if (N2->next != NULL)
//         N1->next = N2->next;
// }

int main()
{
    process *p1;
    p1 = (process *)malloc(sizeof(process));
    strcpy(p1->state, "running");
    p1->id = 1;
    p1->priority = 1;
    p1->arrivalTime = 1;
    p1->remainingTime = 3;
    p1->runTime = 4;
    p1->WaitingTime = 6;
    p1->memsize = 20;

    process *p2;
    p2 = (process *)malloc(sizeof(process));
    strcpy(p1->state, "ready");
    p2->priority = 2;
    p2->id = 2;
    p2->arrivalTime = 10;
    p2->remainingTime = 32;
    p2->runTime = 14;
    p2->WaitingTime = 64;
    p2->memsize = 30;

    process *p3;
    p3 = (process *)malloc(sizeof(process));
    strcpy(p1->state, "running");
    p3->priority = 0;
    p3->id = 3;
    p3->arrivalTime = 11;
    p3->remainingTime = 31;
    p3->runTime = 12;
    p3->WaitingTime = 64;
    p3->memsize = 40;


    process *p4;
    p4 = (process *)malloc(sizeof(process));
    strcpy(p1->state, "running");
    p4->priority = 0;
    p4->id = 4;
    p4->arrivalTime = 11;
    p4->remainingTime = 31;
    p4->runTime = 12;
    p4->WaitingTime = 64;
    p4->memsize = 5;

    MemLinkedList *pq = newMemLinkedList();
    pushMem(&pq, p1, 3);
    pushMem(&pq, p2, 3);
    pushMem(&pq, p3, 3);
    //printMemLinkedList(&pq);
    freeMem(&pq,p2);
    pushMem(&pq, p4, 3);
    printf("======================================\n");
    printMemLinkedList(&pq);
    return 0;
}
