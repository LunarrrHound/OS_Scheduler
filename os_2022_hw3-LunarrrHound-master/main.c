#include <stdlib.h>
#include <stdbool.h>
#include "include/shell.h"
#include "include/command.h"
#include "include/builtin.h"

int main(int argc, char *argv[])
{	
	algo = argv[1];
	init_all();
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
