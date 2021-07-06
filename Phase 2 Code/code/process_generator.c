#include "headers.h"
void clearResources(int);
int msgQ_id;

struct msgbuff
{
    long mtype;
    process p;
    //char mtext[256];
};

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    FILE *inputFile;
    char *inputFileName = argv[1];
    inputFile = fopen(inputFileName, "r"); //Opens file with parameters (path, reading only)
    //Parameters that will be readed from the input file
    int inputFileID;
    int inputFileArrTime;
    int inputFileRunTime;
    int inputFilePriority;
    int inputFileMemSize;
    char buf1[500];
    getcwd(buf1, sizeof(buf1));
    char buf2[500];
    getcwd(buf2, sizeof(buf2));

    const char *pathClk = strcat(buf1, "/clk.out");
    const char *pathScheduler = strcat(buf2, "/scheduler.out");

    int send_val;

    int count = 0;
    char c;
    c = fgetc(inputFile);
    while (c != EOF)
    {
        if (c == '#')
        {
            char buff[255];
            fgets(buff, 255, inputFile);
        }
        else
        {
            count++;
        }
        c = fgetc(inputFile);
    }

    //inputFileId is the number of process
    //process* fileProcesses[count];
    process fileProcesses[count];
    int fileProcessesCount = 0;

    fclose(inputFile); //Closes the inputFile

    inputFile = fopen(inputFileName, "r"); //Opens file with parameters (path, reading only)

    c = fgetc(inputFile);
    while (c != EOF)
    {
        //With every loop, the values of inputFileID, inputFileArrTime, inputFileRunTime and inputFilePriority changes
        //The values are added to the array fileProcesses
        if (c == '#')
        {
            char buff[255];
            fgets(buff, 255, inputFile);
        }
        else
        {
            inputFileID = c - '0'; //To convert '0' --> 0, '1' --> 1 etc.
            fscanf(inputFile, "\t%d\t%d\t%d\t%d\n", &inputFileArrTime, &inputFileRunTime, &inputFilePriority, &inputFileMemSize);
            //process* p = (process*) malloc(sizeof(process));
            process p;
            p.id = inputFileID;
            p.arrivalTime = inputFileArrTime;
            p.runTime = inputFileRunTime;
            p.priority = inputFilePriority;
            p.memsize = inputFileMemSize;
            p.WaitingTime = 0;
            fileProcesses[fileProcessesCount++] = p;
        }
        c = fgetc(inputFile);
    }

    //NOW we have an array of Processes "fileProcesses" with size "fileProcessesCount" that contains the parameters of each process
    // for (int i = 0; i < fileProcessesCount; i++)
    // {
    //     printf("%d\t%d\t%d\t%d\t%d\n",fileProcesses[i].id,fileProcesses[i].arrivalTime,fileProcesses[i].runTime,fileProcesses[i].priority,fileProcesses[i].memsize);
    // }

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int Algorithm = atoi(argv[3]);
    // printf("the number of algorithm: %d\n", Algorithm);
    if (Algorithm < 1 || Algorithm > 5)
    {
        printf("error \n");
    }
    int Quantum = 0;
    if (Algorithm == 5)
        Quantum = atoi(argv[5]);

    // switch (Algorithm)
    // {
    // case 1:
    //     printf("Scheduled Algorithm is First Come First Serve (FCFS)\n");
    //     break;
    // case 2:
    //     printf("Scheduled Algorithm is Shortest Job First (SJF)\n");
    //     break;
    // case 3:
    //     printf("Scheduled Algorithm is Preemptive Highest Priority First (HPF) \n");
    //     break;
    // case 4:
    //     printf("Scheduled Algorithm is Shortest Remaining Time Next \n");
    //     break;
    // case 5:
    //     printf("Scheduled Algorithm is Round Robin (RR)\n");
    //     break;
    // }

    // 3. Initiate and create the scheduler and clock processes.

    int clk_Pid = fork();
    if (clk_Pid == -1)
        return -1;
    else if (clk_Pid == 0) // CLK
    {
        execl(pathClk, "clk.out", NULL);
    }

    int scheduler_Pid = fork();
    if (scheduler_Pid == -1)
        return -1;
    else if (scheduler_Pid == 0) //SCHEDULER
    {
        if (Algorithm != 5)
        {
            execl(pathScheduler, "scheduler.out", argv[3], argv[5], NULL);
        }
        else
        {
            execl(pathScheduler, "scheduler.out", argv[3], argv[5], argv[7], NULL);
        }
    }

    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    printf("Current Time is %d\n", x);
    // TODO Generation Main Loop

    // 5. Create a data structure for processes and provide it with its parameters.

    // 6. Send the information to the scheduler at the appropriate time.
    int k = 0;
    key_t key_id;
    key_id = ftok("keyfile", 65);
    msgQ_id = msgget(key_id, 0666 | IPC_CREAT); // creates or verifies the existence of an up queue with this key_id
    if (msgQ_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    while (k != fileProcessesCount)
    {
        const int currentTime = getClk();
        if (currentTime >= fileProcesses[k].arrivalTime)
        {
            struct msgbuff messageToSend;
            messageToSend.p = fileProcesses[k];
            messageToSend.mtype = 1;
            size_t sz = sizeof(struct msgbuff) - sizeof(long);
            //printf("===============SEND %p\nID= %d",messageToSend.p,messageToSend.p->id);
            send_val = msgsnd(msgQ_id, &messageToSend, sz, !IPC_NOWAIT);
            if (send_val == -1)
                perror("Error in send\n");
            k++;
        }
    }
    //     //send signal to the scheduler that it has finished
    kill(scheduler_Pid, SIGUSR1);
    int stat_loc;
    int sid = wait(&stat_loc);
    while ((stat_loc & 0x00FF)) // wait until the scheduler exits or is destroyed
    {
        int sid = wait(&stat_loc);
    };
    //     // 7. Clear clock resources
    kill(clk_Pid,SIGKILL);
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    msgctl(msgQ_id, IPC_RMID, (struct msqid_ds *)0);
    printf("Clearing resources called\n");
    killpg(getpgrp(), SIGKILL);
    destroyClk(true);
}
