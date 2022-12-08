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
    if(waiting_head == NULL) {
        waiting_head = cur_t;
    }
    else{
        Task *temp = waiting_head;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        cur_t = temp->next;
    }
    cur_t->remain_time = ms;
    // current = NULL;
    // current->state = 2;
    // inq(&waiting_head, &current);
    swapcontext(&current->uctx, &main_process);
}

void task_exit()
{   
    current->state = 3;
    printf("Task %s done\n", current->name);
    current = NULL;
    num_tasks--;
    setcontext(&main_process);
}