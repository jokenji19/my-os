/**
 * @file scheduler.h
 * @brief Basic task scheduler for multitasking demo
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

/* Maximum number of tasks */
#define MAX_TASKS 3

/* Task states */
#define TASK_READY    0
#define TASK_RUNNING  1

/* Simple task structure */
typedef struct {
    int id;
    int state;
    void (*entry_point)(void);
    int runtime_ticks;
} task_t;

/* Function prototypes */
void init_scheduler();
void create_task(int id, void (*entry_point)(void));
void schedule();

#endif
