#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/function.h"
#include "../include/shell.h"

int ready_index = 0;
fun function_list[] = {task1, task2, task3, task4, task5, task6, task7, task8, task9};
const char* task_states[] = {"READY", "RUNNING", "WAITING", "TERMINATED"};

const char *func_name[] = {
	"task1",
	"task2",
	"task3",
	"task4",
	"task5",
	"task6",
	"task7",
	"task8",
	"task9"
};

int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
	printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    	if (chdir(args[1]) != 0)
      		perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i) {
    	if(str[i] >= 48 && str[i] <= 57)
			continue;
        else
		    return false;
  	}
  	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if(strcmp(args[1], "-i") == 0) {

	    pid_t pid = getpid();
	    printf("%d\n", pid);
	
	} else if (strcmp(args[1], "-p") == 0) {
	
		if (args[2] == NULL) {
      		printf("mypid -p: too few argument\n");
      		return 1;
    	}

    	sprintf(fname, "/proc/%s/stat", args[2]);
    	int fd = open(fname, O_RDONLY);
    	if(fd == -1) {
      		printf("mypid -p: process id not exist\n");
     		return 1;
    	}

    	read(fd, buffer, BUF_SIZE);
	    strtok(buffer, " ");
    	strtok(NULL, " ");
	    strtok(NULL, " ");
    	char *s_ppid = strtok(NULL, " ");
	    int ppid = strtol(s_ppid, NULL, 10);
    	printf("%d\n", ppid);
	    
		close(fd);

  	} else if (strcmp(args[1], "-c") == 0) {

		if (args[2] == NULL) {
      		printf("mypid -c: too few argument\n");
      		return 1;
    	}

    	DIR *dirp;
    	if ((dirp = opendir("/proc/")) == NULL){
      		printf("open directory error!\n");
      		return 1;
    	}

    	struct dirent *direntp;
    	while ((direntp = readdir(dirp)) != NULL) {
      		if (!isnum(direntp->d_name)) {
        		continue;
      		} else {
        		sprintf(fname, "/proc/%s/stat", direntp->d_name);
		        int fd = open(fname, O_RDONLY);
        		if (fd == -1) {
          			printf("mypid -p: process id not exist\n");
          			return 1;
        		}

        		read(fd, buffer, BUF_SIZE);
        		strtok(buffer, " ");
        		strtok(NULL, " ");
        		strtok(NULL, " ");
		        char *s_ppid = strtok(NULL, " ");
		        if(strcmp(s_ppid, args[2]) == 0)
		            printf("%s\n", direntp->d_name);

        		close(fd);
     		}
	   	}
    	
		closedir(dirp);
	
	} else {
    	printf("wrong type! Please type again!\n");
  	}
	
	return 1;
}

int add(char **args)
{
	Task *new_task = (Task*)malloc(sizeof(Task));
	if(strcmp(algo, "PP")) { //if not priority based, ignore the priority
		new_task->priority = -1;
	}
	else{
		new_task->priority = atoi(args[3]);
	}
	new_task->next = NULL;
	new_task->remain_time = 0;
	new_task->past_time = 0;
	new_task->running_time = 0;
	new_task->waiting_time = 0;
	char* task_name = args[1];
	char* function_name = args[2];
	strcpy(new_task->name, task_name);
	new_task->state = 0; //READY
	
	new_task->uctx.uc_stack.ss_sp = new_task->stack;
	new_task->uctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
	new_task->uctx.uc_stack.ss_flags = 0;
	new_task->uctx.uc_link = &main_process;
	getcontext(&new_task->uctx);
	for(int i = 0; i < 9; i++) {
		if(!strcmp(function_name, func_name[i])) {
			new_task->function = function_list[i];
			makecontext(&new_task->uctx, function_list[i], 0);
			break;
		}
	}
	ready_index++;
	num_tasks++;
	new_task->task_id = ready_index;
	inq(&ready_head, &new_task);
	add_history(&history_head, &new_task);
	// if(history_head == NULL) {
	// 	history_head = new_task;
	// }
	// else {
	// 	Task *now = history_head;
	// 	while(now->next_his != NULL) {
	// 		now = now->next_his;
	// 	}
	// 	now->next_his = new_task;
	// }
	printf("Task %s is ready\n", task_name);

	return 1;
}

int del(char **args)
{
	deq(&ready_head, args[1]);
	num_tasks--;
	printf("Task %s is killed\n", args[1]);
	return 1;
}

int ps(char **args)
{	
	printf("TID|     name|     state|  runnnig|  waiting| turnaround| resources| priority\n");
	printf("------------------------------------------------------------------------------\n");
	Task *temp = history_head;
	while(temp != NULL) {
		printf("  %d|       %s|   %s|       %d|      %d|          |     %d\n", temp->task_id, temp->name, task_states[temp->state], temp->running_time, temp->waiting_time, temp->priority);
		temp = temp->next_his;
	}
	printf("here\n");
	return 1;
}

int start(char **args)
{
	struct itimerval ovalue, value;
	signal(SIGVTALRM, sighandler);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 10000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 10000; //every 10ms
	setitimer(ITIMER_VIRTUAL, &value, &ovalue);

	getcontext(&main_process);
	printf("Start simulation\n");
	while(num_tasks != 0) {
		if(ready_head != NULL) {
			ready_head->state = 1;
			current = ready_head;
			ready_head = ready_head->next;
			current->next = NULL;
			swapcontext(&main_process, &current->uctx);
		}
	}
	ready_head = NULL;
	waiting_head = NULL;
	printf("Simulation ended\n");
	return 1;
}

const char *builtin_str[] = {
 	"help",
 	"cd",
	"echo",
 	"exit",
 	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start"
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
  	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
