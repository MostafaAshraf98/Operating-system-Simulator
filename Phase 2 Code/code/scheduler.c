#include "headers.h"

struct msgbuff
{
    long mtype;
    process p;
};

typedef struct OutputLine
{
    int time;
    char state[20]; //alocated or running
    int y;          //size of memory
    int process;    //process ID
    int i;          //start memory address
    int j;          //end memory address

} OutputLine;

bool stillSending = true;
int Algorithm;
int Quantum;
int memAlgo;
struct msgbuff messageToReceive;
int currentClk;
int previousClk;
int k;
int shmid1;
//For scheduler.perf
float avgWTA = 0;
float avgWaiting = 0;
float cpuUti;
float cpuRunTime = 0;
FILE *outFile;
MemLinkedList *memLinkedList;
WaitingLinkedList *waitLinkedList;

void handler(int signum) // THe SIGUSER1 signal handler
{
    stillSending = false;
}
void printOutputFile(process *ptr);
void printSchedulerPerf();
bool tryMemPush(process *p);

process *pointerToRunningProcess;
char *pathProcess;
int secretNumber;
PriorityQueue *pq;

int main(int argc, char *argv[])
{
    outFile = fopen("memory.log", "w");
    fprintf(outFile, "%3s\t%4s\t%2s\t%9s\t%3s\t%5s\t%11s\t%2s\t%4s\t%4s\t%2s\t%4s\n", "#At", "time", "x", "allocated", "y", "bytes", "for process", "z", "from", "i", "to", "j");

    //Setting the path for the processes
    char buf1[500];
    getcwd(buf1, sizeof(buf1));
    pathProcess = strcat(buf1, "/process.out");

    //initializing the clock
    initClk();
    previousClk = getClk();
    cpuRunTime = getClk();
    // //array of shared memory addresses with the processes in the scheduler to be able to communicate with them
    // int *arrSharedMemoryIDS = (int *)malloc(sizeof(int));

    // k = 0; // iterator on the array of Addresses and the array of ids
    int quantumCount;

    signal(SIGUSR1, handler);

    memLinkedList = newMemLinkedList();
    waitLinkedList = newWaitingLinkedList();

    //seeting the received arguments from the process generator ( which algorithm and the quantum value in case of RR)
    Algorithm = atoi(argv[1]);
    memAlgo = atoi(argv[2]);
    if (Algorithm == 5) // if RR
    {
        Quantum = atoi(argv[2]);
        quantumCount = Quantum;
        memAlgo = atoi(argv[3]);
    }
    //Creating the message queue to IPC with the process generator
    key_t key_id;
    int msgQ_id;
    int rec_val;
    secretNumber = 65;

    key_id = ftok("keyfile", 65);
    msgQ_id = msgget(key_id, 0666 | IPC_CREAT); // creates or verifies the existence of an up queue with this key_id

    //Setting the priority queue
    pq = newPriorityQueue();

    pointerToRunningProcess = NULL; // pointer to the running process

    //the condition that the scheduer continue scheduling the process
    //this condition only breaks if the process generator send the signal SIGUSR1 that it has sent all the processes
    //in addition to the other condition that there still a running process or processes in the queue
    while (stillSending || pointerToRunningProcess != NULL || !isEmpty(&pq)) //|| !isEmptyWait(&waitLinkedList)
    {

        currentClk = getClk(); // with each iteration i get the current clock

        size_t sz = sizeof(struct msgbuff) - sizeof(long);               // the size of the message sent on the message queue
        rec_val = msgrcv(msgQ_id, &messageToReceive, sz, 0, IPC_NOWAIT); // receives messages with any mtype
        process p = messageToReceive.p;
        strcpy(p.state, messageToReceive.p.state);
        if (rec_val != -1) // if the shcedule did receive a new process successfully
        {
            // printf("Received===================\n");
            tryMemPush(&p);
            // printf("============Printing Queue \n");
            // printQueue(&pq);
            //enqueue(&pq, shmaddr1); // the process is added to the ready queue

            //we need to put the address of the pocess in the shared memory address
            //so that this address points to the process
        }
        if (pointerToRunningProcess == NULL && !isEmpty(&pq)) // if there is not a running process
        {
            pointerToRunningProcess = dequeue(&pq);
            if (strcmp(pointerToRunningProcess->state, "ready") == 0)
            {
                strcpy(pointerToRunningProcess->state, "started");
            }
            else
                strcpy(pointerToRunningProcess->state, "resumed");

            previousClk = getClk();
        }
        if ((currentClk - previousClk) >= 1 && (pointerToRunningProcess != NULL)) // There is a running a running process and a second has passed
        {
            cpuRunTime += (currentClk - previousClk); //a clock has passed and there is a running process
            incrementWaintingTime(&pq);
            incrementWaintingTimeWait(&waitLinkedList);

            if (Algorithm == 5 && pointerToRunningProcess->remainingTime != 0)
            {
                quantumCount--;
                if (quantumCount == 0)
                {
                    strcpy(pointerToRunningProcess->state, "stopped");
                    enqueue(&pq, pointerToRunningProcess);

                    pointerToRunningProcess = dequeue(&pq);
                    if (strcmp(pointerToRunningProcess->state, "ready") == 0)
                        strcpy(pointerToRunningProcess->state, "started");
                    else
                        strcpy(pointerToRunningProcess->state, "resumed");

                    quantumCount = Quantum;
                }
            }

            if (pointerToRunningProcess->remainingTime == 0)
            {
                strcpy(pointerToRunningProcess->state, "finished");
                // printf("Finished and free===================\n");

                Node *ptr1 = waitLinkedList->head;
                Node *prev = NULL;
                while ( ptr1 != NULL)
                {
                    if ( tryMemPush(ptr1->p) )
                    {
                        if ( prev == NULL)
                        {
                            waitLinkedList->head = ptr1->next;

                        }
                        else
                        {
                            prev->next = ptr1->next;
                        }
                    }
                    prev = ptr1;
                    ptr1 = ptr1->next;
                }
                printOutputFile(pointerToRunningProcess);
                int idToremove = pointerToRunningProcess->priority;
                shmctl(idToremove, IPC_RMID, (struct shmid_ds *)0);
                // printf("SHmid removed = %d\n",idToremove);
                freeMem(&memLinkedList, pointerToRunningProcess);
                // printf("===========FreeMemFinished======================\n");
                pointerToRunningProcess = NULL;
                if (Algorithm == 5)
                {
                    quantumCount = Quantum;
                }
            }

            previousClk = getClk();
        }
    }
    fclose(outFile);
    //TODO: upon termination release the clock resources.
    destroyClk(false);
    exit(0);
}

void printOutputFile(process *ptr)
{
    OutputLine out;
    out.time = currentClk;

    if (strcmp(ptr->state, "finished") == 0) ///then memory is freed
        strcpy(out.state, "freed");
    else
        strcpy(out.state, "allocated");

    out.y = ptr->memsize;
    out.process = ptr->id;
    out.i = ptr->memStartAddr;
    out.j = ptr->memEndAddr;

    char buff[20];
    strcpy(buff, out.state);
    // fprintf(outFile, "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d", out.time, out.process, buff, out.arr, out.total, out.remain, out.wait);
    fprintf(outFile, "%3s\t%4s\t%2d\t%9s\t%3d\t%5s\t%11s\t%2d\t%4s\t%4d\t%2s\t%4d\n", "At", "time", out.time, buff, out.y, "bytes", "for process", out.process, "from", out.i, "to", out.j);
}

bool tryMemPush(process *p)
{
    //here we want to make a IPC between process and scheduler (using shared memory)
    key_t key_id;
    key_id = ftok("keyfile", secretNumber);
    shmid1 = shmget(key_id, sizeof(process), IPC_CREAT | 0644);    // create or verify the existence of a shared memory
    //printf("========ID created = %d\n",shmid1);
    process *shmaddr1 = (process *)shmat(shmid1, (process *)0, 0); // attach to shared memory address

    strcpy(shmaddr1->state, "ready");
    shmaddr1->arrivalTime = p->arrivalTime;
    shmaddr1->id = p->id;
    shmaddr1->WaitingTime = p->WaitingTime;
    shmaddr1->runTime = p->runTime;
    shmaddr1->remainingTime = p->runTime;
    shmaddr1->memsize = p->memsize;

    bool memPushed = pushMem(&memLinkedList, shmaddr1, memAlgo);
    if (!memPushed)
    {
        shmctl(shmid1, IPC_RMID, (struct shmid_ds *)0);
        push(&waitLinkedList, p);
        // printf("trymempush false===================\n");
        // printf("=========================================================\n");
        // printMemLinkedList(&memLinkedList);
        // printf("============================================================\n");
        return false;
    }
    else
    {
        char buff[100];
        sprintf(buff, "%d", secretNumber);

        int process_pid = fork(); // forking the process
        if (process_pid == -1)
        {
            perror("error");
        }
        else if (process_pid == 0) //the process
        {
            execl(pathProcess, "process.out", buff, NULL); // We pass the secret number to the process to attach to the same memory address
        }
        secretNumber++; // different secret number for the next process

        switch (Algorithm)
        {
        case 1: // FCFS
            shmaddr1->priority = 0;
            break;
        case 2: //SJF
            shmaddr1->priority = shmaddr1->runTime;
            break;
        case 3: //HptrRecProF   preemptive
            break;
        case 4: //SRTN          preemptive
            shmaddr1->priority = shmaddr1->remainingTime;
            break;
        case 5: //RR            preemptive
            shmaddr1->priority = 0;
            break;
        }

        if (pointerToRunningProcess != NULL)
        {
            switch (Algorithm) // based on algorithm we will decide what happens to the process in running
            {
            case 3:
                if (shmaddr1->priority < pointerToRunningProcess->priority)
                {
                    strcpy(pointerToRunningProcess->state, "stopped");

                    enqueue(&pq, pointerToRunningProcess);

                    pointerToRunningProcess = shmaddr1;
                    strcpy(pointerToRunningProcess->state, "started");
                }
                else
                {
                    //Adding the process to the queue
                    enqueue(&pq, shmaddr1); // the process is added to the ready queue
                }
                break;
            case 4:
                if (shmaddr1->runTime < pointerToRunningProcess->remainingTime)
                {
                    strcpy(pointerToRunningProcess->state, "stopped");

                    enqueue(&pq, pointerToRunningProcess);

                    pointerToRunningProcess = shmaddr1;
                    strcpy(pointerToRunningProcess->state, "started"); //////////////////////////////////////
                }
                else
                {
                    //Adding the process to the queue
                    enqueue(&pq, shmaddr1); // the process is added to the ready queue
                }
                break;
            default:
                enqueue(&pq, shmaddr1); // the process is added to the ready queue
                break;
            }
        }
        else
        {
            enqueue(&pq, shmaddr1); // the process is added to the ready queue
        }
        // printf("trymempush true===================\n");
        printOutputFile(shmaddr1);
        // printf("=========================================================\n");
        // printMemLinkedList(&memLinkedList);
        // printf("============================================================\n");
        return true;
    }
}
