#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "../include/command.h"
#include "../include/builtin.h"
#include "../include/shell.h"

void init_all(){
	num_tasks = 0;
	history_count = 0;
	ready_head = NULL;
	waiting_head = NULL;
	current = NULL;
	history_head = NULL;
	all_available = true;
	for(int i = 0; i < 8; i++) {
		resource_available[i] = true;
	}
}

void routine() {
	Task *temp = waiting_head, *prev = waiting_head;
	if(current != NULL){
		// printf("running adding..\n");
		if(!strcmp(algo, "RR")) {
			// printf("wtf\n");
			Task *cur_t = current;
			cur_t->RR_past += 1;
			if(cur_t->RR_past == 3) {
				cur_t->RR_past = 0;
				cur_t->state = 0;
				inq(&ready_head, &cur_t);
				current = NULL;
				swapcontext(&cur_t->uctx, &main_process);
			}
		}
		else if(!strcmp(algo, "PP")) {
			if(ready_head != NULL && ready_head->priority < current->priority) {
				Task *cur_t = current;
				cur_t->state = 0;//READY
				inq(&ready_head, &cur_t);
				current = NULL;
				swapcontext(&cur_t->uctx, &main_process);
			}
		}
		current->running_time += 1;
		current->past_time += 1;
	}
	while(temp != NULL) {
		all_available = true;
		temp->past_time += 1;
		temp->waiting_time += 1;
		// printf("%s %d\n",temp->name, temp->past_time);
		if(temp->remain_time == -1) {//waiting for resources
			for(int i = 0; i < temp->resource_num; i++) {
				int target = temp->need[i];
				if(resource_available[target] == false) {
					all_available = false;
					break;
				}
			}
			if(all_available == true) { //move from waiting queue to ready queue
				printf("all avail\n");
				if(temp == waiting_head) {
					waiting_head = temp->next;
				}
				else{
					prev->next = temp->next;
				}
				temp->state = 0;
				temp->next = NULL;
				inq(&ready_head, &temp);
			}
		}
		else{//waiting for sleep time up
			if(temp->past_time >= temp->remain_time) {
				temp->past_time = 0;
				temp->remain_time = 0;
				//remove from waiting queue to ready queue
				if(temp == waiting_head) {
					waiting_head = temp->next;
				}else{
					prev->next = temp->next;
				}
				temp->state = 0;
				temp->next = NULL;
				inq(&ready_head, &temp);
			}
		}
		prev = temp;
		temp = temp->next;
	}
	return ;
} 

void pause_process() {
	isPause = true;
	if(current) {
		swapcontext(&current->uctx, &main_process);
	}
	else{
		setcontext(&main_process);
	}
	return ;
}

void sighandler(int signo) {
	switch(signo){
		case SIGVTALRM:
			// printf("catched a signal\n");
			//do something
			routine();
			break;
	}
	return ;
}

void add_wait(Task **head, Task **node) {
	Task *temp = (*head);
	if(temp == NULL) {
		(*head) = (*node), (*node)->next = NULL;
	}
	else{
		while(temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = (*node);
	}
	(*node)->next = NULL;
	return ;
}

void add_history(Task **head, Task **node) {
	Task *temp = (*head);
	if(temp == NULL) {
		(*head) = (*node), (*node)->next_his = NULL;
	}
	else {
		while(temp->next_his != NULL) {
			temp = temp->next_his;
		}
		temp->next_his = (*node);
		(*node)->next_his = NULL;
	}
	return ;
}

void deq(Task **head, char* name) {
	Task *temp = (*head), *prev = NULL;
	
	if(temp != NULL && !strcmp(temp->name, name)){
		(*head)->state = 3;
		(*head) = temp->next;
		return ;
	}
	while(temp != NULL && strcmp(temp->name, name)) {
		prev = temp;
		temp = temp->next;
	}
	if(temp == NULL) return ;
	temp->state = 3;
	prev->next = temp->next;
	temp->next = NULL;
}

void inq(Task **head, Task **node){
	Task *temp = (*head), *prev = NULL;
	if(!temp) {
		(*head) = (*node), (*node)->next = NULL;
	}
	else {
		while(temp) {
			if(temp->priority <= (*node)->priority) {
				prev = temp, temp = temp->next;
			}
			else {
				(*node)->next = temp;
				if(temp != (*head)) {
					prev->next = (*node);
				}
				else{
					(*head) = (*node);
				}
				break;
			}
		}
		if(!temp) {
			prev->next = (*node);
		}
	}
	return ;
}

char *read_line()
{
    char *buffer = (char *)malloc(BUF_SIZE * sizeof(char));
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        exit(1);
    }

	if (fgets(buffer, BUF_SIZE, stdin) != NULL) {
		if (buffer[0] == '\n' || buffer[0] == ' ' || buffer[0] == '\t') {
			free(buffer);
			buffer = NULL;
		} else {
			if (strncmp(buffer, "replay", 6) == 0) {
				char *token = strtok(buffer, " ");
				token = strtok(NULL, " ");
				int index = strtol(token, NULL, 10);
				if (index > MAX_RECORD_NUM || index > history_count) {
					free(buffer);
					buffer = NULL;
				} else {
					char *temp = (char *)malloc(BUF_SIZE * sizeof(char));
					int head = 0;
					if (history_count > MAX_RECORD_NUM) {
						head += history_count % MAX_RECORD_NUM;
					}
					strncpy(temp, history[(head + index - 1) % MAX_RECORD_NUM], BUF_SIZE);
					token = strtok(NULL, " ");
					while (token) {
						strcat(temp, " ");
						strcat(temp, token);
						token = strtok(NULL, " ");
					}
					strncpy(buffer, temp, BUF_SIZE);
					free(temp);
					buffer[strcspn(buffer, "\n")] = 0;
					strncpy(history[history_count % MAX_RECORD_NUM], buffer, BUF_SIZE);
					++history_count;
				}
			} else {
				buffer[strcspn(buffer, "\n")] = 0;
				strncpy(history[history_count % MAX_RECORD_NUM], buffer, BUF_SIZE);
				++history_count;
			}
		}
	}

	return buffer;
}

struct cmd *split_line(char *line)
{
	int args_length = 10;
    struct cmd *new_cmd = (struct cmd *)malloc(sizeof(struct cmd));
    new_cmd->head = (struct pipes *)malloc(sizeof(struct pipes));
    new_cmd->head->args = (char **)malloc(args_length * sizeof(char *));
	for (int i = 0; i < args_length; ++i)
		new_cmd->head->args[i] = NULL;
    new_cmd->head->length = 0;
    new_cmd->head->next = NULL;
    new_cmd->background = false;
    new_cmd->in_file = NULL;
    new_cmd->out_file = NULL;

	struct pipes *temp = new_cmd->head;
    char *token = strtok(line, " ");
    while (token != NULL) {
        if (token[0] == '|') {
            struct pipes *new_pipe = (struct pipes *)malloc(sizeof(struct pipes));
			new_pipe->args = (char **)malloc(args_length * sizeof(char *));
			for (int i = 0; i < args_length; ++i)
				new_pipe->args[i] = NULL;
			new_pipe->length = 0;
			new_pipe->next = NULL;
			temp->next = new_pipe;
			temp = new_pipe;
        } else if (token[0] == '<') {
			token = strtok(NULL, " ");
            new_cmd->in_file = token;
        } else if (token[0] == '>') {
			token = strtok(NULL, " ");
            new_cmd->out_file = token;
        } else if (token[0] == '&') {
            new_cmd->background = true;
        } else {
			temp->args[temp->length] = token;
			temp->length++;
        }
        token = strtok(NULL, " ");
    }

    return new_cmd;
}

void test_cmd_struct(struct cmd *cmd)
{
	struct pipes *temp = cmd->head;
	int pipe_count = 0;
	while (temp != NULL) {
		printf("pipe %d: ", pipe_count);
		for (int i = 0; i < temp->length; ++i) {
			printf("%s ", temp->args[i]);
		}
		printf("\n");
		temp = temp->next;
		++pipe_count;
	}
	printf(" in: %s\n", cmd->in_file ? cmd->in_file : "none");
	printf("out: %s\n", cmd->out_file ? cmd->out_file : "none");
	printf("background: %s\n", cmd->background ? "true" : "false");
}
