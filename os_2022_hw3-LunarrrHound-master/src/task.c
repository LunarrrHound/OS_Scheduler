#include "../include/task.h"
#include "../include/builtin.h"
#include <ucontext.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/command.h"

void task_sleep(int ms)
{
    Task *cur_t = current;
    cur_t->state = 2;//WAITING STATE
    cur_t->remain_time = ms;
    cur_t->next = NULL;
    inq(&waiting_head, &cur_t);
    printf("%s goes to sleep\n", cur_t->name);
    current = NULL;
    swapcontext(&cur_t->uctx, &main_process);
}

void task_exit()
{   
    current->state = 3;
    printf("Task %s done\n", current->name);
    current = NULL;
    num_tasks--;
    setcontext(&main_process);
}