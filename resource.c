#include "../include/resource.h"
#include "../include/builtin.h"
#include "../include/command.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void get_resources(int count, int *resources)
{   
    Task *cur_t = current;
    all_available = true;
    cur_t->resource_num = count;
    for(int i = 0; i < count; i++) {
        int target = resources[i];
        if(resource_available[target] == false) {//not available
            all_available = false;
            cur_t->remain_time = -1;
            cur_t->state = 2;
            for(int i = 0; i < count; i++) {
                cur_t->need[i] = resources[i];
            }
            printf("Task %s is waiting for resources\n", cur_t->name);
            add_wait(&waiting_head, &cur_t);
            current = NULL;
            swapcontext(&cur_t->uctx, &main_process);
            break;
        }
    }
    if(all_available == true) {
        for(int i = 0; i < count; i++) {
            int target = resources[i];
            printf("Task %s getting resource %d\n", cur_t->name, target);
            resource_available[target] = false;
        }
    }
    return ;
}

void release_resources(int count, int *resources)
{
    for(int i = 0; i < count; i++) {
        int target = resources[i];
        if(resource_available[target] == false) {
            resource_available[target] = true;
            printf("Task %s release resource %d\n", current->name, target);
        }
    }
}
