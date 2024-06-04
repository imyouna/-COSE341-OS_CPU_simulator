CPU scheduling simulator

<variables>
job[]: after create processes, save in job[]
jobQ[]: job Queue of each scheduling (initialize every scheduling)
readyQ[]: ready Queue of each scheduling (initialize every scheduling)
waitQ[]: waiting Queue of each scheduling (initialize every scheduling)
terminateQ[]: terminated processes in this Queue (initialize every scheduling)
ganttChart[]: gantt chart of each scheduling (initialize every scheduling)

* every queue is circular queue
* degree of multiprogramming is 100
* 


<functions>
- init_queue: reset queue
- init_jobQ: reset jobQ and copy job -> jobQ
- init_ganttChart: reset gantt chart
- add_ganttChart: add gantt chart entry
  if same process => do nothing
  if different process => update end time, start new process & time
  if terminate all processes => update end time
- auto_create_process: create processes automatically(randomly)
- create_process: create processes what user wants to make
- IOProcess: all processes in waitQ decrease their IO burst remain
  if IO burst remain == 0 => move into readyQ
- FCFS_scheduling: scheduling processes with FCFS algorithm
- SJF_scheduling: scheduling processes with SJF algorithm
- SRJF_scheduling: scheduling processes with SJF algorithm and preemption
- priority_scheduling: scheduling processes with priority algorithm
- premptive_priority_scheduling: scheduling processes with priority algorithm and preemption
- RR_shceduling: scheduling processes with round robin algorithm
- premptive_priority_RR_scheduling: scheduling processes with priority algorithm and preemption
  if same priority in readyQ => scheduling with round robin algorithm
  
