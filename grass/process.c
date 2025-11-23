/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for process management
 */

#include "process.h"

extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    static uint curr_pid = 0;
    for (uint i = 1; i <= MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            /* Student's code goes here (Preemptive Scheduler | System Call). */

            /* Initialization of lifecycle statistics, MLFQ or process sleep. */
            proc_set[i].creation_time = mtime_get();
            proc_set[i].first_schedule_time = 0;  /* Not yet scheduled */
            proc_set[i].total_cpu_time = 0;
            proc_set[i].timer_interrupt_count = 0;
            //Initialize MLFQ fields - all processes start at level 0
            proc_set[i].mlfq_level = 0;
            proc_set[i].level_runtime = 0;
            proc_set[i].last_schedule_time = 0;

            /* Student's code ends here. */
            return curr_pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */

    /* Print the lifecycle statistics of the terminated process or processes. */
    if (pid != GPID_ALL) {
        /* Find the process and print its statistics */
        for (uint i = 0; i < MAX_NPROCESS; i++) {
            if (proc_set[i].pid == pid && proc_set[i].status != PROC_UNUSED) {
                ulonglong current_time = mtime_get();
                ulonglong turnaround_time = current_time - proc_set[i].creation_time;
                ulonglong response_time = proc_set[i].first_schedule_time > 0 ? 
                    proc_set[i].first_schedule_time - proc_set[i].creation_time : 0;
                INFO("Process %d terminated:", pid);
                INFO("  Turnaround Time: %d ms", (int)(turnaround_time / 1000));
                INFO("  Response Time: %d ms", (int)(response_time / 1000));
                INFO("  CPU Time: %d ms", (int)(proc_set[i].total_cpu_time / 1000));
                INFO("  Timer Interrupts: %d", (int)proc_set[i].timer_interrupt_count);
                INFO("  Final MLFQ Level: %d", (int)proc_set[i].mlfq_level);
                earth->mmu_free(pid);
                proc_set_status(pid, PROC_UNUSED);
                break;
            }
        }
    } else {
        /* Free all user processes. */
        for (uint i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                ulonglong current_time = mtime_get();
                ulonglong turnaround_time = current_time - proc_set[i].creation_time;
                ulonglong response_time = proc_set[i].first_schedule_time > 0 ? 
                    proc_set[i].first_schedule_time - proc_set[i].creation_time : 0;
                INFO("Process %d terminated:", proc_set[i].pid);
                INFO("  Turnaround Time: %d ms", (int)(turnaround_time / 1000));
                INFO("  Response Time: %d ms", (int)(response_time / 1000));
                INFO("  CPU Time: %d ms", (int)(proc_set[i].total_cpu_time / 1000));
                INFO("  Timer Interrupts: %d", (int)proc_set[i].timer_interrupt_count);
                INFO("  Final MLFQ Level: %d", (int)proc_set[i].mlfq_level);
                
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }
    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */
    /* Update the MLFQ-related fields in struct process* p after this
     * process has run on the CPU for another runtime microseconds. */
    uint old_level = p->mlfq_level;
    p->level_runtime += runtime;
    ulonglong level_time_quantum = MLFQ_LEVEL_RUNTIME(p->mlfq_level);
    while (p->level_runtime >= level_time_quantum && p->mlfq_level < MLFQ_NLEVELS - 1) {
        p->level_runtime -= level_time_quantum;
        p->mlfq_level++;
        level_time_quantum = MLFQ_LEVEL_RUNTIME(p->mlfq_level);
        if (p->pid > GPID_SHELL) {
            //INFO("PID %d: promoted to level %d, carryover runtime: %d ms",p->pid, (int)p->mlfq_level, (int)(p->level_runtime / 1000));
        }
    }
     /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduler). */
    // to handle keyboard input - reset shell to level 0
    if (!earth->tty_input_empty()) {
        /* Reset the level of GPID_SHELL if there is pending keyboard input. */
        for (uint i = 0; i < MAX_NPROCESS; i++) {
            if (proc_set[i].pid == GPID_SHELL && proc_set[i].status != PROC_UNUSED) {
                proc_set[i].mlfq_level = 0;
                proc_set[i].level_runtime = 0;
                break;
            }
        }
    }
    /* Reset the level of all processes every MLFQ_RESET_PERIOD microseconds. */
    static ulonglong MLFQ_last_reset_time = 0;
    ulonglong current_time = mtime_get();
    //Initializing on first call 
    if (MLFQ_last_reset_time == 0) {
        MLFQ_last_reset_time = current_time;
        return;
    }
    ulonglong elapsed = current_time - MLFQ_last_reset_time; // calculating elapsed time
    if (elapsed >= MLFQ_RESET_PERIOD) {
        //INFO("[MLFQ] Priority boost at time %d ms (elapsed: %d ms)",(int)(current_time / 1000), (int)(elapsed / 1000));
        for (uint i = 0; i < MAX_NPROCESS; i++) {
            if (proc_set[i].status != PROC_UNUSED) {
                proc_set[i].mlfq_level = 0;           /* Reset all processes to level 0 */
                proc_set[i].level_runtime = 0;
            }
        }
        
        /*Updating the last reset time to current time */
        MLFQ_last_reset_time = current_time;
    }
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    /* Update the sleep-related fields in the struct process for process pid. */

    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print out the pid of the process running on each CPU core. */

    /* Student's code ends here. */
}
