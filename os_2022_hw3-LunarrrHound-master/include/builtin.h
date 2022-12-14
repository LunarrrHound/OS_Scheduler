#ifndef BUILTIN_H
#define BUILTIN_H
#include <ucontext.h>
#include <stdbool.h>
#define DEFAULT_STACK_SIZE 1024*128
#define MAX_TASK_SIZE 1024
int help(char **args);
int cd(char **args);
int echo(char **args);
int exit_shell(char **args);
int record(char **args);
int mypid(char **args);

int add(char **args);
int del(char **args);
int ps(char **args);
int start(char **args);

extern const char *builtin_str[];

extern const int (*builtin_func[]) (char **);

extern int num_builtins();

ucontext_t main_process;

typedef void (*fun)(void);

typedef struct Task{
    ucontext_t uctx;
    int state;
    char stack[DEFAULT_STACK_SIZE];
    char name[50];
    int task_id;
    int priority;
    fun function;
    int remain_time;
    int past_time;
    int running_time;
    int waiting_time;
    int need[8];
    int resource_num;
    int RR_past;
    int turnaround;
    struct Task *next;
    struct Task *next_his;
}Task;

struct Task *history_head;
struct Task *ready_head;
struct Task *waiting_head;
struct Task *current;
int num_tasks;
bool isPause;
bool all_available;
bool resource_available[8];

#endif
