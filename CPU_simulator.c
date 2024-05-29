#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define DEGREE_OF_MP 100

typedef struct process
{
  int pid;
  int CPUburst;
  int IOburst;
  int IOstart;
  int arrival;
  int tmpArrival;
  int priority;
  int CPUburst_remain;
  int IOburst_remain;
  int timequantum;
  int waitingTime;
  int turnaroundTime;
  int responseTime;

} Process;
typedef Process *pProcess;

typedef struct ganttChatEntry
{
  int pid;
  int start;
  int end;
} GanttchatEntry;

// variables
pProcess job[DEGREE_OF_MP]; // create process and save in "job"
int job_front, job_rear, job_size;

pProcess jobQ[DEGREE_OF_MP]; // jobQueue in each scheduling (Copy job to jobQ per scheduling)
int jobQ_front, jobQ_rear, jobQ_size;

pProcess readyQ[DEGREE_OF_MP];
int readyQ_front, readyQ_rear, readyQ_size;

pProcess waitQ[DEGREE_OF_MP];
int waitQ_front, waitQ_rear, waitQ_size;

pProcess terminateQ[DEGREE_OF_MP];
int terminateQ_front, terminateQ_rear, terminateQ_size;

GanttchatEntry ganttChat[DEGREE_OF_MP * 50];
int ganttChat_idx;

// functions
void init_queue(pProcess *queue, int *front, int *rear, int *size);
void init_jobQ(int numProcess);
void init_ganttChat();
void add_ganttChat(int pid, int time);
void print_ganttChat();

void create_process(int numProcess, int tq);
void enqueue(pProcess *queue, pProcess newProcess, int *rear, int *size);
pProcess dequeue(pProcess *queue, int *front, int *size);
int IOProcess(int time); // IO operation

int arrivalCompare(const void *a, const void *b);
int IOremainCompare(const void *a, const void *b);
int CPUremainCompare(const void *a, const void *b);
int priorityCompare(const void *a, const void *b);

void print_terminateQ();
void print_process();
void print_readyQ();

void FCFS_scheduling(int numProcess);
void SJF_sort();
void SJF_scheduling(int numProcess);
void SRJF_scheduling(int numProcess);

void priority_sort();
void priority_scheduling(int numProcess);
void premptive_priority_scheduling(int numProcess);
void RR_sheduling(int numProcess, int timequantum);

//
void init_queue(pProcess *queue, int *front, int *rear, int *size)
{
  *front = 0;
  *rear = 0;
  *size = 0;

  for (int i = 0; i < DEGREE_OF_MP; i++)
  {
    queue[i] = NULL;
  }
}

void init_jobQ(int numProcess)
{
  jobQ_front = job_front;
  jobQ_rear = job_rear;
  jobQ_size = job_size;

  for (int i = 0; i < numProcess; i++)
  {
    pProcess copyProcess = (pProcess)malloc(sizeof(Process));
    *copyProcess = *job[i];
    jobQ[i] = copyProcess;
  }
}

void init_ganttChat()
{
  ganttChat_idx = 0;
  for (int i = 0; i < DEGREE_OF_MP * 50; i++)
  {
    ganttChat[i].pid = -1;
    ganttChat[i].start = 0;
    ganttChat[i].end = 0;
  }
}

void add_ganttChat(int pid, int time)
{
  if (ganttChat[ganttChat_idx].pid == pid)
  {
    return;
  }
  else if (pid == -2)
  { // finish signal
    ganttChat[ganttChat_idx].end = time;
  }
  else
  {
    ganttChat[ganttChat_idx].end = time;
    ganttChat_idx++;
    ganttChat[ganttChat_idx].pid = pid;
    ganttChat[ganttChat_idx].start = time;
    return;
  }
}

void print_ganttChat()
{
  printf("<<Ghant Chat>>\n");
  for (int i = 0; i <= ganttChat_idx; i++)
  {
    if (ganttChat[i].start != ganttChat[i].end)
    {
      if (ganttChat[i].pid == -1)
      { // idle
        printf("[%d--idle--%d]\n", ganttChat[i].start, ganttChat[i].end);
      }
      else
      {
        printf("[%d--P%d--%d]\n", ganttChat[i].start, ganttChat[i].pid, ganttChat[i].end);
      }
    }
  }
}

void print_process()
{
  printf("----------create process----------\n");
  int i = job_front;
  while (i != job_rear)
  {
    printf("P%d arrival time: %d, CPU burst time: %d (after %d I/O start), IO burst time: %d,  priority: %d\n", job[i]->pid, job[i]->arrival, job[i]->CPUburst, job[i]->IOstart, job[i]->IOburst, job[i]->priority);
    i = (i + 1) % DEGREE_OF_MP;
  }
  printf("\n");
}

void create_process(int numProcess, int timequantum)
{
  init_queue(job, &job_front, &job_rear, &job_size);

  srand((unsigned int)time(NULL));
  for (int i = 0; i < numProcess; i++)
  {
    pProcess newProcess = (pProcess)malloc(sizeof(Process));
    newProcess->pid = i;
    newProcess->CPUburst = (rand() % 10) + 2; // 2<=CPU burst time<=26
    newProcess->IOburst = rand() % 10;        // 0<=IO burst time<=24;
    if (newProcess->IOburst > 0)
      newProcess->IOstart = rand() % ((newProcess->CPUburst) - 1) + 1; // CPU - I/O - CPU
    else
      newProcess->IOstart = 0;
    newProcess->arrival = rand() % (numProcess + 1);
    newProcess->tmpArrival = newProcess->arrival;
    newProcess->priority = rand() % numProcess + 1; // 1<=priority<=numProcess
    newProcess->timequantum = timequantum;

    newProcess->CPUburst_remain = newProcess->CPUburst;
    newProcess->IOburst_remain = newProcess->IOburst;
    newProcess->turnaroundTime = 0;
    newProcess->waitingTime = 0;

    enqueue(job, newProcess, &job_rear, &job_size);
  }
  print_process();
}

void enqueue(pProcess *queue, pProcess newProcess, int *rear, int *size)
{
  queue[*rear] = newProcess;
  *rear = ((*rear) + 1) % DEGREE_OF_MP;
  (*size)++;
}

pProcess dequeue(pProcess *queue, int *front, int *size)
{
  if (*size == 0)
  {
    // printf("dqueue fail!!\n");
    return NULL;
  }
  else
  {
    pProcess pop = queue[*front];
    queue[*front] = NULL;
    *front = ((*front) + 1) % DEGREE_OF_MP;
    (*size)--;
    return pop;
  }
}

void print_readyQ()
{
  printf("<ready Queue>\n");
  int i = readyQ_front;
  while (i != readyQ_rear)
  {
    printf("pid: %d arrival: %d\n", readyQ[i]->pid, readyQ[i]->arrival);
    i = (i + 1) % DEGREE_OF_MP;
  }
  printf("\n");
}

void SJF_sort()
{
  if (readyQ_size < 2)
  {
    return;
  }
  else
  {
    int i = readyQ_front;
    int j = 0;
    pProcess tmp[DEGREE_OF_MP] = {
        NULL,
    };
    while (i != readyQ_rear)
    {
      tmp[j] = readyQ[i];
      j++;
      i = (i + 1) % DEGREE_OF_MP;
    }
    qsort(tmp, j, sizeof(pProcess), CPUremainCompare);

    i = readyQ_front;
    j = 0;
    while (i != readyQ_rear)
    {
      readyQ[i] = tmp[j];
      j++;
      i = (i + 1) % DEGREE_OF_MP;
    }
  }
}

void priority_sort()
{
  if (readyQ_size < 2)
  {
    return;
  }
  else
  {
    int i = readyQ_front;
    int j = 0;
    pProcess tmp[DEGREE_OF_MP] = {
        NULL,
    };
    while (i != readyQ_rear)
    {
      tmp[j] = readyQ[i];
      j++;
      i = (i + 1) % DEGREE_OF_MP;
    }
    qsort(tmp, j, sizeof(pProcess), priorityCompare);

    i = readyQ_front;
    j = 0;
    while (i != readyQ_rear)
    {
      readyQ[i] = tmp[j];
      j++;
      i = (i + 1) % DEGREE_OF_MP;
    }
  }
}

void print_terminateQ()
{
  // printf("<terminated>\n");
  int i = terminateQ_front;
  double avg_turnaround = 0;
  double avg_waiting = 0;
  while (i != terminateQ_rear)
  {
    // printf("P%d turnaround: %d waiting time: %d\n", terminateQ[i]->pid, terminateQ[i]->turnaroundTime, terminateQ[i]->waitingTime);
    avg_turnaround += terminateQ[i]->turnaroundTime;
    avg_waiting += terminateQ[i]->waitingTime;
    i = (i + 1) % DEGREE_OF_MP;
  }
  printf("average turnaround: %lf, average waiting: %lf\n", avg_turnaround / terminateQ_size, avg_waiting / terminateQ_size);
}

int IOProcess(int time)
{
  int done = 0;         // exist IO done process
  int cnt = waitQ_size; // to ensure we only loop through the existing elements
  while (cnt > 0)
  {
    pProcess currentProcess = dequeue(waitQ, &waitQ_front, &waitQ_size);
    // printf("IO operation: P%d remain IO burst %d\n", currentProcess->pid, currentProcess->IOburst_remain);

    currentProcess->IOburst_remain--;

    if (currentProcess->IOburst_remain <= 0)
    {
      currentProcess->tmpArrival = time;
      enqueue(readyQ, currentProcess, &readyQ_rear, &readyQ_size);
      // printf("time: %d, IO done: P%d go to ready queue remain CPU: %d\n", time, currentProcess->pid, currentProcess->CPUburst_remain);
      //  currentProcess->tmpArrival = time;
      done = 1;
    }
    else
    {
      enqueue(waitQ, currentProcess, &waitQ_rear, &waitQ_size); // put back in waitQ if not done
      // printf("time: %d P%d IO remain: %d\n", time, currentProcess->pid, currentProcess->IOburst_remain);
    }
    cnt--;
  }
  return done;
}

int arrivalCompare(const void *a, const void *b)
{
  const pProcess *pa = (const pProcess *)a;
  const pProcess *pb = (const pProcess *)b;

  return (*pa)->arrival - (*pb)->arrival;
}

int IOremainCompare(const void *a, const void *b)
{
  const pProcess *pa = (const pProcess *)a;
  const pProcess *pb = (const pProcess *)b;

  return (*pa)->IOburst_remain - (*pb)->IOburst_remain;
}

int CPUremainCompare(const void *a, const void *b)
{
  const pProcess *pa = (const pProcess *)a;
  const pProcess *pb = (const pProcess *)b;

  return (*pa)->CPUburst_remain - (*pb)->CPUburst_remain;
}

int priorityCompare(const void *a, const void *b)
{
  const pProcess *pa = (const pProcess *)a;
  const pProcess *pb = (const pProcess *)b;

  return (*pa)->priority - (*pb)->priority;
}

void FCFS_scheduling(int numProcess)
{
  init_ganttChat();
  init_jobQ(numProcess);
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  int finishtime = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);
  ;

  printf("----------FCFS scheduling----------\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      // printf("time: %d submission P%d \n", time, submission->pid);
    }
    IOProcess(time);
    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      // printf("time: %d start P%d\n", time, runProcess->pid);
      add_ganttChat(runProcess->pid, time);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      // printf("time: %d idle\n", time);
      idle++;
      add_ganttChat(-1, time);
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      // printf("time: %d P%d\n", time, runProcess->pid);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart) // IO operation
      {
        // printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);
        // runProcess->tmpArrival=time;

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        // printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else if (numTerminate != numProcess)
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
    }
    time++;
  }
  finishtime = time;
  add_ganttChat(-2, time); // pid==-2: finish signal

  print_ganttChat();
  print_terminateQ();
  printf("CPU total time: %d, idle time: %d\n", time, idle);
  printf("CPU utilization: %lf\n\n", (double)(finishtime - idle) / finishtime);
}

void SJF_scheduling(int numProcess)
{
  init_ganttChat();
  init_jobQ(numProcess);
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  int finishtime = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);
  ;

  printf("----------non preemptive SJF scheduling----------\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      SJF_sort();
      // printf("time: %d submission P%d \n", time, submission->pid);
    }
    IOProcess(time);
    SJF_sort();
    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      // printf("time: %d start P%d\n", time, runProcess->pid);
      add_ganttChat(runProcess->pid, time);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      // printf("time: %d idle\n", time);
      idle++;
      add_ganttChat(-1, time);
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      // printf("time: %d P%d\n", time, runProcess->pid);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart) // IO operation
      {
        // printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        // printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else if (numTerminate != numProcess)
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
    }
    time++;
  }
  finishtime = time;
  add_ganttChat(-2, time); // pid==-2: finish signal

  print_ganttChat();
  print_terminateQ();
  printf("CPU total time: %d, idle time: %d\n", time, idle);
  printf("CPU utilization: %lf\n\n", (double)(finishtime - idle) / finishtime);
}

void SRJF_scheduling(int numProcess)
{
  init_ganttChat();
  init_jobQ(numProcess);
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  int finishtime = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);
  ;

  printf("----------preemptive SJF(SRJF) scheduling----------\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      SJF_sort();
      // printf("time: %d submission P%d \n", time, submission->pid);
    }
    IOProcess(time);
    SJF_sort();
    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      // printf("time: %d start P%d\n", time, runProcess->pid);
      add_ganttChat(runProcess->pid, time);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      // printf("time: %d idle\n", time);
      idle++;
      add_ganttChat(-1, time);
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      // printf("time: %d P%d remain CPU: %d\n", time, runProcess->pid, runProcess->CPUburst_remain);

      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart)
      { // IO operation
        // printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        // printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);

        SJF_sort();
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else if (numTerminate != numProcess)
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (readyQ_size > 0 && (runProcess->CPUburst_remain > readyQ[readyQ_front]->CPUburst_remain))
      { // preemption
        enqueue(readyQ, runProcess, &readyQ_rear, &readyQ_size);
        runProcess->tmpArrival = time;
        // printf("time: %d preemptive P%d ", time, runProcess->pid);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        (runProcess->waitingTime) += time - (runProcess->tmpArrival);
        SJF_sort();
        // printf("start new P%d remain CPU: %d\n", runProcess->pid, runProcess->CPUburst_remain);
        add_ganttChat(runProcess->pid, time);
      }
    }
    time++;
  }
  finishtime = time;
  add_ganttChat(-2, time); // pid==-2: finish signal

  print_ganttChat();
  print_terminateQ();
  printf("CPU total time: %d, idle time: %d\n", time, idle);
  printf("CPU utilization: %lf\n\n", (double)(finishtime - idle) / finishtime);
}

void priority_scheduling(int numProcess)
{ // lower priority number, higher priority
  init_ganttChat();
  init_jobQ(numProcess);
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  int finishtime = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);

  printf("----------non preemptive priority scheduling----------\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      priority_sort();
      // printf("time: %d submission P%d \n", time, submission->pid);
    }
    IOProcess(time);
    priority_sort();
    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      // printf("time: %d start P%d\n", time, runProcess->pid);
      add_ganttChat(runProcess->pid, time);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      // printf("time: %d idle\n", time);
      idle++;
      add_ganttChat(-1, time);
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      // printf("time: %d P%d\n", time, runProcess->pid);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart)
      { // IO operation
        // printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        // printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else if (numTerminate != numProcess)
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
    }
    time++;
  }
  finishtime = time;
  add_ganttChat(-2, time); // pid==-2: finish signal

  print_ganttChat();
  print_terminateQ();
  printf("CPU total time: %d, idle time: %d\n", time, idle);
  printf("CPU utilization: %lf\n\n", (double)(finishtime - idle) / finishtime);
}

void premptive_priority_scheduling(int numProcess)
{ // lower priority number, higher priority
  init_ganttChat();
  init_jobQ(numProcess);
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  int finishtime = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);

  printf("----------preemptive priority scheduling----------\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      priority_sort();
      // printf("time: %d submission P%d \n", time, submission->pid);
    }
    IOProcess(time);
    priority_sort();
    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      // printf("time: %d start P%d\n", time, runProcess->pid);
      add_ganttChat(runProcess->pid, time);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      // printf("time: %d idle\n", time);
      idle++;
      add_ganttChat(-1, time);
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      // printf("time: %d P%d remain CPU: %d\n", time, runProcess->pid, runProcess->CPUburst_remain);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart)
      { // IO operation
        // printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        // printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);

        priority_sort();
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          add_ganttChat(runProcess->pid, time);
        }
        else if (numTerminate != numProcess)
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (readyQ_size > 0 && (runProcess->priority > readyQ[readyQ_front]->priority))
      { // preemption
        enqueue(readyQ, runProcess, &readyQ_rear, &readyQ_size);
        runProcess->tmpArrival = time;
        // printf("time: %d preemptive P%d ", time, runProcess->pid);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        (runProcess->waitingTime) += time - (runProcess->tmpArrival);
        priority_sort();
        // printf("start new P%d remain CPU: %d\n", runProcess->pid, runProcess->CPUburst_remain);
        add_ganttChat(runProcess->pid, time);
      }
    }
    time++;
  }
  finishtime = time;
  add_ganttChat(-2, time); // pid==-2: finish signal

  print_ganttChat();
  print_terminateQ();
  printf("CPU total time: %d, idle time: %d\n", time, idle);
  printf("CPU utilization: %lf\n\n", (double)(finishtime - idle) / finishtime);
}

void RR_sheduling(int numProcess, int timequantum)
{
  init_ganttChat();
  init_jobQ(numProcess);
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  int finishtime = 0;
  pProcess runProcess = NULL;

  int tq = timequantum;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);

  printf("----------Round Robin scheduling----------\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      // printf("time: %d submission P%d \n", time, submission->pid);
    }
    IOProcess(time);
    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      runProcess->timequantum = tq;
      // printf("time: %d start P%d\n", time, runProcess->pid);
      add_ganttChat(runProcess->pid, time);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      // printf("time: %d idle\n", time);
      idle++;
      add_ganttChat(-1, time);
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      runProcess->timequantum = runProcess->timequantum - 1;
      // printf("time: %d P%d\n", time, runProcess->pid);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart) // IO operation
      {
        // printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        runProcess->timequantum = tq; // timequantum reset
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          runProcess->timequantum = tq;
          add_ganttChat(runProcess->pid, time);
        }
        else
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        // printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          // printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
          runProcess->timequantum = tq;
          add_ganttChat(runProcess->pid, time);
        }
        else if (numTerminate != numProcess)
        {
          idle++;
          add_ganttChat(-1, time);
        }
      }
      else if (runProcess->timequantum == 0 && readyQ_size > 0) // time slice expired
      {                                                         // Q: ... if P3=4s, tq=2s, p3(1)-p3(2)-p3(3, ready empty)-p4 submission -> p3? or premptive p4? A:... p3
        // printf("time: %d P%d timelice expired ", time, runProcess->pid);
        runProcess->tmpArrival = time;
        enqueue(readyQ, runProcess, &readyQ_rear, &readyQ_size);

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        runProcess->timequantum = tq;
        runProcess->waitingTime += time - (runProcess->tmpArrival);
        // printf("start P%d\n", runProcess->pid);
        add_ganttChat(runProcess->pid, time);
      }
    }
    time++;
  }
  finishtime = time;
  add_ganttChat(-2, time); // pid==-2: finish signal

  print_ganttChat();
  print_terminateQ();
  printf("CPU total time: %d, idle time: %d\n", time, idle);
  printf("CPU utilization: %lf\n", (double)(finishtime - idle) / finishtime);
}

int main(int argc, char *argv[])
{
  int numProcess, tq;

  printf("Welcome to CPU Scheduling Simulator!\n");
  printf("Type process number: ");
  scanf("%d", &numProcess);
  if (numProcess > DEGREE_OF_MP || numProcess < 0)
  {
    printf("invalid number\n");
    return 0;
  }
  printf("Time quantum: ");
  scanf("%d", &tq);
  if (tq <= 0)
  {
    printf("invalid number\n");
    return 0;
  }

  create_process(numProcess, tq);
  FCFS_scheduling(numProcess);
  SJF_scheduling(numProcess);
  SRJF_scheduling(numProcess);
  priority_scheduling(numProcess);
  premptive_priority_scheduling(numProcess);
  RR_sheduling(numProcess, tq);

  return 0;
}
