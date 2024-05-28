#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define INF 1000000
#define DEGREE_OF_MP 100

#define ARRIVAL 0
#define IOREMAIN 1
#define CPUREMAIN 2
#define PRIORITY 3
#define PID 4

#define FCFS 0
#define SJF 1
#define PRI 2
#define PRESJF 3
#define PREPRI 4
#define RR 5
#define MULTI 6

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

// variables
pProcess job[DEGREE_OF_MP];
int job_front, job_rear, job_size;

pProcess jobQ[DEGREE_OF_MP];
int jobQ_front, jobQ_rear, jobQ_size;

pProcess readyQ[DEGREE_OF_MP];
int readyQ_front, readyQ_rear, readyQ_size;

pProcess waitQ[DEGREE_OF_MP];
int waitQ_front, waitQ_rear, waitQ_size;

pProcess terminateQ[DEGREE_OF_MP];
int terminateQ_front, terminateQ_rear, terminateQ_size;

// functions
void init_queue(pProcess *queue, int *front, int *rear, int *size);
void init_jobQ();
void create_process(int numProcess, int tq);
void enqueue(pProcess *queue, pProcess newProcess, int *rear, int *size);
pProcess dequeue(pProcess *queue, int *front, int *size);
int IOProcess(int time); // IO operation

int arrivalCompare(const void *a, const void *b);
int IOremainCompare(const void *a, const void *b);
int CPUremainCompare(const void *a, const void *b);

void print_terminateQ();
void print_jobQ();
void print_readyQ();

void FCFS_scheduling(int numProcess);
void SJF_scheduling(int numProcess);

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

void init_jobQ()
{
  jobQ_front = job_front;
  jobQ_rear = job_rear;
  jobQ_size = job_size;

  for (int i = 0; i < DEGREE_OF_MP; i++)
  {
    jobQ[i] = job[i];
  }
}

void print_jobQ()
{
  printf("<job Queue>\n");
  int i = jobQ_front;
  while (i != jobQ_rear)
  {
    printf("pid: %d arrival: %d CPU burst: %d, IO burst: %d, IO start: %d\n", jobQ[i]->pid, jobQ[i]->arrival, jobQ[i]->CPUburst, jobQ[i]->IOburst, jobQ[i]->IOstart);
    i = (i + 1) % DEGREE_OF_MP;
  }
  printf("jobQ size: %d\n", jobQ_size);
  printf("\n");
}

void create_process(int numProcess, int tq)
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

    newProcess->CPUburst_remain = newProcess->CPUburst;
    newProcess->IOburst_remain = newProcess->IOburst;
    newProcess->turnaroundTime = 0;
    newProcess->waitingTime = 0;

    enqueue(job, newProcess, &job_rear, &job_size);
  }
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

void print_terminateQ()
{
  printf("<terminated>\n");
  int i = terminateQ_front;
  double avg_turnaround = 0;
  double avg_waiting = 0;
  while (i != terminateQ_rear)
  {
    printf("P%d turnaround: %d waiting time: %d\n", terminateQ[i]->pid, terminateQ[i]->turnaroundTime, terminateQ[i]->waitingTime);
    avg_turnaround += terminateQ[i]->turnaroundTime;
    avg_waiting += terminateQ[i]->waitingTime;
    i = (i + 1) % DEGREE_OF_MP;
  }
  printf("average turnaround: %lf, average waiting: %lf\n\n", avg_turnaround / terminateQ_size, avg_waiting / terminateQ_size);
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
      enqueue(readyQ, currentProcess, &readyQ_rear, &readyQ_size);
      printf("time: %d, IO done: P%d go to ready queue size of ready queue: %d\n", time, currentProcess->pid, readyQ_size);
      currentProcess->tmpArrival = time;
      done = 1;
    }
    else
    {
      enqueue(waitQ, currentProcess, &waitQ_rear, &waitQ_size); // put back in waitQ if not done
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

void FCFS_scheduling(int numProcess)
{
  init_jobQ();
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), arrivalCompare);
  print_jobQ();

  printf("<FCFS scheduling>\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      printf("time: %d submission P%d \n", time, submission->pid);
    }

    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      printf("time: %d start P%d\n", time, runProcess->pid);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      printf("time: %d idle\n", time);
      idle++;
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      printf("time: %d P%d\n", time, runProcess->pid);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart) // IO operation
      {
        printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);
        // runProcess->tmpArrival=time;

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
        }
        else
        {
          idle++;
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
        }
        else if (numTerminate != numProcess)
          idle++;
      }
    }
    time++;
    IOProcess(time); // IO operate
  }
  // printf("\n<<all process ended>>\n");
  print_terminateQ();
  printf("CPU idle: %d\n", idle);
}

void SJF_scheduling(int numProcess)
{
  init_jobQ();
  init_queue(readyQ, &readyQ_front, &readyQ_rear, &readyQ_size);
  init_queue(waitQ, &waitQ_front, &waitQ_rear, &waitQ_size);
  init_queue(terminateQ, &terminateQ_front, &terminateQ_rear, &terminateQ_size);

  int numTerminate = 0; // number of terminated process
  int time = 0;
  int idle = 0;
  pProcess runProcess = NULL;

  qsort(jobQ, numProcess, sizeof(pProcess), CPUremainCompare);
  print_jobQ();

  printf("<FCFS scheduling>\n");
  while (numTerminate != numProcess)
  {
    while ((jobQ_size > 0) && (jobQ[jobQ_front]->arrival == time))
    {
      pProcess submission = dequeue(jobQ, &jobQ_front, &jobQ_size);
      enqueue(readyQ, submission, &readyQ_rear, &readyQ_size); // submission
      printf("time: %d submission P%d \n", time, submission->pid);
    }

    if (readyQ_size > 0 && runProcess == NULL)
    {
      runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
      (runProcess->waitingTime) += time - (runProcess->tmpArrival);
      printf("time: %d start P%d\n", time, runProcess->pid);
    }
    else if (readyQ_size == 0 && runProcess == NULL)
    {
      printf("time: %d idle\n", time);
      idle++;
    }
    else if (runProcess != NULL)
    {
      runProcess->CPUburst_remain = runProcess->CPUburst_remain - 1;
      printf("time: %d P%d\n", time, runProcess->pid);
      if (runProcess->IOburst_remain && (runProcess->CPUburst) - (runProcess->CPUburst_remain) == runProcess->IOstart) // IO operation
      {
        printf("time: %d P%d go to waiting queue \n", time, runProcess->pid);
        enqueue(waitQ, runProcess, &waitQ_rear, &waitQ_size);
        // runProcess->tmpArrival=time;

        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);
        if (runProcess != NULL)
        {
          printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
        }
        else
        {
          idle++;
        }
      }
      else if (runProcess->CPUburst_remain == 0) // terminate
      {
        numTerminate++;
        printf("time: %d terminate P%d\n", time, runProcess->pid);
        runProcess->turnaroundTime = time - (runProcess->arrival);
        enqueue(terminateQ, runProcess, &terminateQ_rear, &terminateQ_size);
        runProcess = dequeue(readyQ, &readyQ_front, &readyQ_size);

        if (runProcess != NULL)
        {
          printf("time: %d start new process: %d\n", time, runProcess->pid);
          (runProcess->waitingTime) += time - (runProcess->tmpArrival);
        }
        else if (numTerminate != numProcess)
          idle++;
      }
    }
    time++;
    IOProcess(time); // IO operate
  }
  // printf("\n<<all process ended>>\n");
  print_terminateQ();
  printf("CPU idle: %d\n", idle);
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

  create_process(numProcess, 0);
  FCFS_scheduling(numProcess);
  //SJF_scheduling(numProcess);
  // PRI_alg(num_process);
  // PRESJF_alg(num_process);
  // PREPRI_alg(num_process);
  // RR_alg(num_process, tq);
  // MULTI_Q(num_process, tq);
  // TotalEval();

  return 0;
}
