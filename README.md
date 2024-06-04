# CPU Scheduling Simulator

This project is a CPU scheduling simulator that supports multiple scheduling algorithms. It includes various queues for job management and maintains a Gantt chart to visualize the scheduling process.

## Typedef

### Process

```c
typedef struct {
    int pid;                // Process ID (0 to n-1)
    int CPUburst;           // Total CPU burst time
    int IOburst;            // Total I/O burst time
    int IOstart;            // After 'IOstart' CPU burst time, I/O operation started
    int arrival;            // Arrival time of process
    int tmpArrival;         // Recent time when process goes to readyQ
    int priority;           // Priority of process
    int CPUburst_remain;    // Remaining CPU burst time
    int IOburst_remain;     // Remaining I/O burst time
    int timequantum;        // Remaining time quantum of process
    int waitingTime;        // Waiting time of process
    int turnaroundTime;     // Turnaround time of process
} Process, *pProcess;
```

## Variables

- `job[]`: Stores processes after creation.
- `jobQ[]`: Job queue for each scheduling (initialized at the start of each scheduling).
- `readyQ[]`: Ready queue for each scheduling (initialized at the start of each scheduling).
- `waitQ[]`: Waiting queue for each scheduling (initialized at the start of each scheduling).
- `terminateQ[]`: Queue for terminated processes (initialized at the start of each scheduling).
- `ganttChart[]`: Gantt chart for each scheduling (initialized at the start of each scheduling).

**Note**: Every queue is a circular queue. The degree of multiprogramming is 100.

## Functions

- `init_queue`: Resets the queue.
- `init_jobQ`: Resets `jobQ` and copies `job` to `jobQ`.
- `init_ganttChart`: Resets the Gantt chart.
- `add_ganttChart`: Adds an entry to the Gantt chart.
  - If the same process, does nothing.
  - If a different process, updates the end time, starts a new process, and time.
  - If all processes are terminated, updates the end time.
- `auto_create_process`: Automatically (randomly) creates processes.
- `create_process`: Creates processes manually as specified by the user.
- `IOProcess`: All processes in `waitQ` decrease their I/O burst time.
  - If I/O burst time reaches 0, the process moves to `readyQ`.
- `FCFS_scheduling`: Schedules processes using the FCFS algorithm.
- `SJF_scheduling`: Schedules processes using the SJF algorithm.
- `SRJF_scheduling`: Schedules processes using the SJF algorithm with preemption.
- `priority_scheduling`: Schedules processes using the priority algorithm.
- `preemptive_priority_scheduling`: Schedules processes using the priority algorithm with preemption.
- `RR_scheduling`: Schedules processes using the round-robin algorithm.
- `preemptive_priority_RR_scheduling`: Schedules processes using the priority algorithm with preemption.
  - If processes have the same priority in `readyQ`, schedules using the round-robin algorithm.

## Scheduling Algorithms

- FCFS (First-Come, First-Served)
- SJF (Shortest Job First)
  - Non-preemptive
  - Preemptive
- Priority
  - Non-preemptive
  - Preemptive with FCFS
  - Preemptive with Round Robin
- Round Robin

## How to Run

1. Open Oracle VM VirtualBox.
2. Log in to the OS-tutorial1.
3. Open SSH using VSCode.
4. Compile the simulator:
   ```sh
   gcc -o CPU_simulator CPU_simulator.c
   ```
5. Run the simulator:
   ```sh
   ./CPU_simulator
   ```

6. Follow the prompts:
   - Input the number of processes.
   - Input the time quantum.
   - Input the mode:
     - `0`: Create processes manually.
     - `1`: Create processes automatically (randomly).

## Output

- Prints all created processes.
  - If I/O operation: After n CPU burst time, starts I/O operation.
    - Example: `CPU burst time: 3 (after 2 I/O start), IO burst time: 5`
      - Execution: `CPU 2 -> I/O 5 -> CPU 1`
- Displays the Gantt chart, average waiting time, turnaround time, and CPU utilization for each scheduling.
