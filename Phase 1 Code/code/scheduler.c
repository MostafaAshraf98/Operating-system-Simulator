#include "headers.h"

struct msgbuff
{
    long mtype;
    process p;
};

typedef struct OutputLine
{
    int time;
    int process;
    char state[20];
    int arr;
    int total;
    int remain;
    int wait;
    int TA;    //turnarount time
    float WTA; // weighted turnaround time

} OutputLine;

bool stillSending = true;
int Algorithm;
int Quantum;
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

void handler(int signum) // THe SIGUSER1 signal handler
{
    stillSending = false;
}
void printOutputFile(process *ptr);
void printSchedulerPerf();

int main(int argc, char *argv[])
{
    outFile = fopen("scheduler.log", "w");
    fprintf(outFile, "#At\ttime\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\tk\n");


    //Setting the path for the processes
    char buf1[500];
    getcwd(buf1, sizeof(buf1));
    const char *pathProcess = strcat(buf1, "/process.out");

    //initializing the clock
    initClk();
    previousClk = getClk();
    cpuRunTime = getClk();
    //array of shared memory addresses with the processes in the scheduler to be able to communicate with them
    int *arrSharedMemoryIDS = (int *)malloc(sizeof(int));

    k = 0; // iterator on the array of Addresses and the array of ids
    int quantumCount;

    signal(SIGUSR1, handler);

    //seeting the received arguments from the process generator ( which algorithm and the quantum value in case of RR)
    Algorithm = atoi(argv[1]);
    if (Algorithm == 5) // if RR
    {
        Quantum = atoi(argv[2]);
        quantumCount = Quantum;
    }

    //Creating the message queue to IPC with the process generator
    key_t key_id;
    int msgQ_id;
    int rec_val;
    int secretNumber = 65;

    key_id = ftok("keyfile", 65);
    msgQ_id = msgget(key_id, 0666 | IPC_CREAT); // creates or verifies the existence of an up queue with this key_id

    //Setting the priority queue
    PriorityQueue *pq = newPriorityQueue();

    process *pointerToRunningProcess = NULL; // pointer to the running process

    //the condition that the scheduer continue scheduling the process
    //this condition only breaks if the process generator send the signal SIGUSR1 that it has sent all the processes
    //in addition to the other condition that there still a running process or processes in the queue
    while (stillSending || pointerToRunningProcess != NULL || !isEmpty(&pq))
    {
        currentClk = getClk(); // with each iteration i get the current clock

        size_t sz = sizeof(struct msgbuff) - sizeof(long);               // the size of the message sent on the message queue
        rec_val = msgrcv(msgQ_id, &messageToReceive, sz, 0, IPC_NOWAIT); // receives messages with any mtype
        process p = messageToReceive.p;
        strcpy(p.state, messageToReceive.p.state);
        if (rec_val != -1) // if the shcedule did receive a new process successfully
        {
            //here we want to make a IPC between process and scheduler (using shared memory)
            key_t key_id;
            key_id = ftok("keyfile", secretNumber);
            shmid1 = shmget(key_id, sizeof(process), IPC_CREAT | 0644);   // create or verify the existence of a shared memory
            process *shmaddr1 = (process *)shmat(shmid1, (process *)0, 0); // attach to shared memory address

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

            strcpy(shmaddr1->state, "ready");
            shmaddr1->arrivalTime = p.arrivalTime;
            shmaddr1->id = p.id;
            shmaddr1->WaitingTime = 0;
            shmaddr1->runTime = p.runTime;
            shmaddr1->remainingTime = p.runTime;

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
                        printOutputFile(pointerToRunningProcess);
                        enqueue(&pq, pointerToRunningProcess);

                        pointerToRunningProcess = shmaddr1;
                        strcpy(pointerToRunningProcess->state, "started");
                        printOutputFile(pointerToRunningProcess);
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
                        printOutputFile(pointerToRunningProcess);
                        enqueue(&pq, pointerToRunningProcess);

                        pointerToRunningProcess = shmaddr1;
                        strcpy(pointerToRunningProcess->state, "started"); //////////////////////////////////////
                        printOutputFile(pointerToRunningProcess);
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
            // printf("============Printing Queue \n");
            // printQueue(&pq);
            //enqueue(&pq, shmaddr1); // the process is added to the ready queue

            //we need to put the address of the pocess in the shared memory address
            //so that this address points to the process
            arrSharedMemoryIDS[k] = shmid1;
            k++;
        }
        if (pointerToRunningProcess == NULL && !isEmpty(&pq)) // if there is not a running process
        {
            pointerToRunningProcess = dequeue(&pq);
            if (strcmp(pointerToRunningProcess->state, "ready") == 0)
                strcpy(pointerToRunningProcess->state, "started");
            else
                strcpy(pointerToRunningProcess->state, "resumed");

            printOutputFile(pointerToRunningProcess);

            previousClk = getClk();
        }
        if ((currentClk - previousClk) >= 1 && (pointerToRunningProcess != NULL)) // There is a running a running process and a second has passed
        {
            cpuRunTime += (currentClk - previousClk); //a clock has passed and there is a running process
            incrementWaintingTime(&pq);

            if (Algorithm == 5 && pointerToRunningProcess->remainingTime != 0)
            {
                quantumCount--;
                if (quantumCount == 0)
                {
                    strcpy(pointerToRunningProcess->state, "stopped");
                    printOutputFile(pointerToRunningProcess);
                    enqueue(&pq, pointerToRunningProcess);

                    pointerToRunningProcess = dequeue(&pq);
                    if (strcmp(pointerToRunningProcess->state, "ready") == 0)
                        strcpy(pointerToRunningProcess->state, "started");
                    else
                        strcpy(pointerToRunningProcess->state, "resumed");

                    printOutputFile(pointerToRunningProcess);

                    quantumCount = Quantum;
                }
            }

            if (pointerToRunningProcess->remainingTime == 0)
            {
                strcpy(pointerToRunningProcess->state, "finished");
                printOutputFile(pointerToRunningProcess);
                int idToremove = pointerToRunningProcess->priority;
                shmctl(idToremove, IPC_RMID, (struct shmid_ds *)0);
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
    printSchedulerPerf();
    //TODO: upon termination release the clock resources.
    destroyClk(false);
    exit(0);
}

void printOutputFile(process *ptr)
{
    OutputLine out;
    out.time = currentClk;
    out.process = ptr->id;
    strcpy(out.state, ptr->state);
    out.arr = ptr->arrivalTime;
    out.total = ptr->runTime + ptr->WaitingTime;
    out.remain = ptr->remainingTime;
    out.wait = ptr->WaitingTime;

    char buff[20];
    strcpy(buff, out.state);
    fprintf(outFile, "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d", out.time, out.process, buff, out.arr, out.total, out.remain, out.wait);

    if (strcmp(out.state, "finished") == 0)
    {
        out.TA = (currentClk - out.arr);
        out.WTA = (float)out.TA / (float)ptr->runTime;
        avgWTA += out.WTA;
        avgWaiting += out.wait;
        fprintf(outFile, "\tTA\t%d\tWTA\t%.2f", out.TA, out.WTA);
    }

    fprintf(outFile, "\n");
}

void printSchedulerPerf()
{
    FILE *outPerf;
    outPerf = fopen("scheduler.perf", "w");

    cpuUti = cpuRunTime * 100 / currentClk;

    fprintf(outPerf, "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.2f", cpuUti, avgWTA / k, avgWaiting / k);

    fclose(outPerf);
}
