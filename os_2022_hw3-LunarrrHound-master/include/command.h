#ifndef COMMAND_H
#define COMMAND_H

#define MAX_RECORD_NUM 16
#define BUF_SIZE 1024

#include <stdbool.h>
#include "../include/builtin.h"

struct pipes {
	char **args;
	int length;
	struct pipes *next;
};

struct cmd {
	struct pipes *head;
	bool background;
    char *in_file, *out_file;
};

char *history[MAX_RECORD_NUM];
int history_count;

char *read_line();
struct cmd *split_line(char *);
void test_cmd_struct(struct cmd *);
void inq(Task**, Task**);
void deq(Task**, char*);
void sighandler(int);
void routine();
void add_history(Task**, Task**);
void add_wait(Task**, Task**);
void pause_process();
void init_all();

#endif
