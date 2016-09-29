#include <stdio.h>
#include <signal.h>
#include "bool.h"
#include "input_parser.h"

void ignore (int sig){
    // do nothing
}
int main() {
    printf("Free Shell started\n");
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
    if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
	printf("Shell started\n");
	context con;
	context_init(&con);
	while (true){
		char *line = readline("$ ");
		if (line == NULL) break;
		add_history(line);
		if (strcmp(line, "exit") == 0) break;
		parse_input_line(line, &con);
		free(line);
	}
	context_dispose(&con);
    return 0;
}