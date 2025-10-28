/**
 * @file scheduler.c
 * @brief Basic round-robin task scheduler implementation
 */

#include "scheduler.h"
#include "kernel.h"

/* Global task array and current task index */
static task_t tasks[MAX_TASKS];
static int current_task = 0;
static int task_count = 0;

/* Fake processes that demonstrate multitasking */
void task_process_1() {
    /* Process 1: Changes border color */
    static int color = VGA_COLOR_BLUE;
    vga_print("Task 1 running - Border Blue", 0, 20, VGA_COLOR_WHITE);
    color = (color == VGA_COLOR_BLUE) ? VGA_COLOR_GREEN : VGA_COLOR_BLUE;
    vga_print("", 50, 20, color); // Draw a colored block

    vga_print("Next: Task 2 (Red)", 0, 22, VGA_COLOR_RED);
}

void task_process_2() {
    /* Process 2: Shows counter */
    static int counter = 0;
    counter++;
    char buffer[16];
    itoa(counter, buffer, 10);

    vga_print("Task 2 running - Counter: ", 0, 20, VGA_COLOR_WHITE);
    vga_print(buffer, 26, 20, VGA_COLOR_RED);
    vga_print("Next: Task 3 (Yellow)", 0, 22, VGA_COLOR_YELLOW);
}

void task_process_3() {
    /* Process 3: Shows memory usage */
    static int mem_pos = 0;
    vga_print("Task 3 running - Memory Monitor", 0, 20, VGA_COLOR_WHITE);
    vga_print("[", 30 + mem_pos, 20, VGA_COLOR_YELLOW);
    mem_pos = (mem_pos + 1) % 20;
    vga_print("]", 40, 20, VGA_COLOR_YELLOW);
    vga_print("Next: Task 1 (Blue)", 0, 22, VGA_COLOR_BLUE);
}

/* Initialize the scheduler */
void init_scheduler() {
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        tasks[i].id = -1;
        tasks[i].state = TASK_READY;
        tasks[i].entry_point = 0;  /* NULL not defined in freestanding C */
        tasks[i].runtime_ticks = 0;
    }
    task_count = 0;
}

/* Create a new task */
void create_task(int id, void (*entry_point)(void)) {
    if (task_count < MAX_TASKS) {
        tasks[task_count].id = id;
        tasks[task_count].state = TASK_READY;
        tasks[task_count].entry_point = entry_point;
        tasks[task_count].runtime_ticks = 0;
        task_count++;
    }
}

/* Schedule next task - called from timer interrupt */
void schedule() {
    if (task_count == 0) return;

    /* Clear previous task display area */
    vga_print("                           ", 0, 20, VGA_COLOR_BLACK);
    vga_print("                           ", 0, 22, VGA_COLOR_BLACK);

    /* Run current task */
    tasks[current_task].runtime_ticks++;
    if (tasks[current_task].entry_point != 0) {
        tasks[current_task].entry_point();
    }

    /* Schedule next task (round-robin) */
    current_task = (current_task + 1) % task_count;
}
