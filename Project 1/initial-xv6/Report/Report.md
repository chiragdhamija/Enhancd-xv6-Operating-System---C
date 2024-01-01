# XV6 Scheduling Algorithms

This repository contains implementations of three scheduling algorithms: First-Come-First-Serve (FCFS), Multi-Level Feedback Queue (MLFQ), and Round Robin in the XV6 operating system. While FCFS and MLFQ were implemented by me, Round Robin is provided as the default scheduling algorithm. You can select a specific scheduling algorithm at compile time using the Makefile variable SCHEDULER. For instance, to compile for MLFQ, you can use 'make qemu SCHEDULER=MLFQ'. Please note that only one scheduling algorithm can be active at a time.

## FCFS (First-Come-First-Serve)

### Overview

The First-Come, First-Served (FCFS) scheduling algorithm is a straightforward and foundational approach to process management. It operates by prioritizing processes for execution in the order in which they enter the ready state, adhering to the principle of handling tasks based solely on their arrival sequence. This method exemplifies the fundamental concept of processing tasks in the same order they become ready for execution, making FCFS a cornerstone of scheduling algorithms.

### Implementation

In XV6, FCFS is implemented in following steps:

Step 1: We utilize a previously introduced variable in xv6, namely "uint ctime," which holds the timestamp denoting when the process was initially created.

Step 2: In the scheduler() function located in the kernel/proc.c file, we identify the process with the earliest creation time by iterating through all the runnable processes. This selected process is stored in the process_to_run variable. Subsequently, we transition to the chosen process using the swtch function. Prior to this context switch, we update the state of the chosen process to "running" and assign it as the current CPU's active process :
        #if defined FCFS
        struct proc *p;
        struct cpu *c = mycpu();
        c->proc = 0;
        struct proc *process_to_run;

        for (;;)
        {
            process_to_run = 0;
            // Avoid deadlock by ensuring that devices can interrupt.
            intr_on();

            for (p = proc; p < &proc[NPROC]; p++)
            {
            acquire(&p->lock);
            if (p->state != RUNNABLE)
            {
                continue;
            }
            if (process_to_run == 0 || p->ctime < process_to_run->ctime)
            {
                process_to_run = p;
            }
            }
            for (p = proc; p < &proc[NPROC]; p++)
            {
            if (p == process_to_run)
            {
                continue;
            }
            release(&p->lock);
            }
            if (process_to_run != 0 && process_to_run->state == RUNNABLE)
            {
            process_to_run->state = RUNNING;
            c->proc = process_to_run;
            swtch(&c->context, &process_to_run->context);
            c->proc = 0;
            release(&process_to_run->lock);
            }
        }
        

Step 3: Subsequently, we must render it non-preemptive by deactivating I/O within schedulertest.c. Additionally, within the usertrap function found in kernel/trap.c, we should explicitly state that when FCFS (First-Come-First-Served) is specified, the yield() function should not be invoked.

Step 4: Subsequently, you may initiate the scheduler test by compiling it with the "FCFS" flag.


## MLFQ (Multi-Level-FeedBack-Queue)

### Overview

The Multi-Level Feedback Queue (MLFQ) is a scheduling algorithm employed in operating systems. It allocates varying priorities to processes and dynamically adjusts these priorities based on their behavior and usage patterns.

MLFQ scheduling consistently prioritizes and executes processes in the highest priority queue. It operates as a preemptive scheduling algorithm, meaning that if, during a clock tick, it identifies a process with a higher priority waiting to be scheduled, it will interrupt the current process.

Furthermore, if a process voluntarily yields control of the CPU before utilizing all its allocated time slices, it remains within its current queue. Additionally, MLFQ scheduling incorporates a process aging mechanism. Each process accumulates a tick count, reflecting the time it has spent waiting in the queue. If this count surpasses a predefined threshold, the process is moved to the back of a higher-priority queue. This mechanism effectively mitigates the risk of process starvation.

### Implementation

Step 1: Within the struct proc defined in proc.h, introduce three additional variables, namely "waiting_time" (indicating the duration a process remains pending before its scheduling), "pqueue_num" (denoting the priority queue number, with 0 signifying the highest priority group to which the process is assigned), and "running_ticks" (reflecting the quantity of execution cycles the process undergoes while it is in RUNNING state).

Step 2: Commence by initializing the aforementioned variables within the "found" segment of the allocproc() function, which can be found in proc.c. Set waiting_time to zero, running_ticks to zero, and pqueue_num to zero. This initial configuration is employed because every newly created process is initially assigned to the highest-priority queue.

Step 3: Adjust the waiting time of all processes in the RUNNABLE state, signifying those prepared for execution but currently not active.
        void update_time()
        {
        struct proc *p;
        for (p = proc; p < &proc[NPROC]; p++)
        {
            acquire(&p->lock);
            if (p->state == RUNNING)
            {
            p->rtime++;
            // p->running_ticks++;
            }
            else if(p->state==RUNNABLE)
            {
            p->waiting_time++;
            }
            release(&p->lock);
        }

Step 4: Within the "scheduler()" function located in the "proc.c" file, create a variable named "process_to_run" with the purpose of storing the process designated for scheduling. Enable interrupts to prevent potential deadlock situations.

Step 5: To incorporate aging, proceed by iterating through all the active processes in the "RUNNABLE" state. Examine whether the elapsed waiting time of a process surpasses the predefined starvation threshold (in this case, 40 seconds). If the threshold is exceeded, elevate the priority of the process by moving it to a higher-priority queue. Additionally, reset the process's waiting time and running ticks to 0.
            for (p = proc; p < &proc[NPROC]; p++)
                {
                if (p->state == RUNNABLE)
                {
                    if (p->waiting_time >= 40 && p->pqueue_num != 0)
                    {
                    p->pqueue_num--;
                    p->running_ticks = 0;
                    p->waiting_time=0;
                    }
                }
                }

Step 6: In this step, the task is to identify a process for scheduling. This is accomplished by iterating through all the processes that are in a RUNNABLE state. The selection process involves two criteria: firstly, the process must belong to the highest priority bracket among all the processes (indicated by a lower pqueue_num). Secondly, the process with the maximum waiting_time is given preference as it would have been placed at the front of the queue when assuming a queue data structure. It is important to note that the acquire and release steps can be skipped  here, as it is presumed that MLFQ will operate on a single CPU.
    for (p = proc; p < &proc[NPROC]; p++)
        {
        acquire(&p->lock);
        if (p->state != RUNNABLE)
        {
            continue;
        }
        if (process_to_run == 0)
        {
            process_to_run = p;
        }
        else if (process_to_run->pqueue_num > p->pqueue_num)
        {
            process_to_run = p;
        }
        else if (process_to_run->pqueue_num == p->pqueue_num && process_to_run->waiting_time < p->waiting_time)
        {
            process_to_run = p;
        }
        }
        for (p = proc; p < &proc[NPROC]; p++)
        {
        if (p == process_to_run)
        {
            continue;
        }
        release(&p->lock);
        }

Step 7: Capture the identified process within the variable 'process_to_run' and execute a context switch utilizing the 'swtch' function. Before doing so, ensure that the state of this process is set to 'RUNNING' and that the 'waiting_time' is reset to zero. This adjustment is necessary since the process has now been scheduled, and the waiting time will commence from zero until its subsequent scheduling.
    if (process_to_run != 0 && process_to_run->state == RUNNABLE)
    {
      process_to_run->waiting_time = 0;
      process_to_run->state = RUNNING;
      c->proc = process_to_run;
      swtch(&c->context, &process_to_run->context);
      c->proc = 0;
      release(&process_to_run->lock);
    }

Step 8: Within the "usertrap()" function in the "trap.c" file, whenever a timer interrupt occurs, specifically when the "which_dev" variable becomes equal to 2, we increment the "running_ticks" counter for the RUNNING process by 1 to signify its current execution. Following this, we proceed to assess whether the accumulated running time of the process, as denoted by the "running_ticks" function, exceeds the time slice allocated for the respective queue to which the process belongs. It is to be noted that the assumed time slice values for the four queues are as follows: queue 0 (1 tick), queue 1 (3 ticks), queue 2 (9 ticks), and queue 3 (15 ticks).
        #ifdef MLFQ
            p->running_ticks++;
            int time_slice = 0;
            if (p->pqueue_num == 0)
            {
            time_slice = 1;
            }
            else if (p->pqueue_num == 1)
            {
            time_slice = 3;
            }
            else if (p->pqueue_num == 2)
            {
            time_slice = 9;
            }
            else if (p->pqueue_num == 3)
            {
            time_slice = 15;
            }

            if (p->running_ticks > time_slice)
            {
            p->pqueue_num++;
            if (p->pqueue_num > 3)
            {
                p->pqueue_num = 3;
            }
            p->running_ticks = 0;
            yield();
            }

Step 9: If the running time exceed the allocated time slice, it indicates that the process has consumed more time than allotted within its current queue(time slice). Consequently, we shall reduce its priority level by incrementing the queue number by 1 and reset its running_ticks to 0. This action will facilitate the process's termination through the yield function.

Step 10: In the event that the elapsed time is equal to or less than the allocated time slice, we refrain from yielding and permit the process to maintain its current priority.


Step 11: Subsequently, you may initiate the scheduler test by compiling it with the "MLFQ" flag.

## Comparision between schedulers

| Scheduling Algorithm  | Run Time | Wait Time |
|-----------------------|----------|-----------|
| RR                    | 11       | 145       |
| RR (CPU-2)            | 11       | 120       |
| FCFS                  | 22       | 102       |
| FCFS (CPU-2)          | 25       | 49        |
| MLFQ                  | 11       | 140       |

You can find the graphs for MLFQ in the same directory with MLFQ with CPU=1 and aging time 28 and 40 ticks.


In the case of my specific laptop, there are instances when the runtime values significantly escalate owing to a CPU-related concern. In order to address this issue, I employ the method of restarting my Linux system.


    
    
