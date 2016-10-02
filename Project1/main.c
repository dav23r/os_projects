#include <stdio.h>
#include <signal.h>
#include "functions.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "bool.h"
#include "input_parser.h"
#include "load_functions.h"

void ignore (int sig){
    // do nothing
}
char * def_prompt(){
	char * t = getcwd(NULL, 0);
	char * p = "$: ";
	if(t != NULL){
		char *prompt = malloc(sizeof(char) * (strlen(t) + strlen(p) + 1));
		if(prompt == NULL){
			free(t);
			return NULL;
		}
		(*prompt) = '\0';
		strcat(prompt, t);
		free(t);
		strcat(prompt, p);
		return prompt;
	}
	else return strdup(p);
}
int main(int argc, char** argv) {
    printf("Free Shell started\n");
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
    if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);

	context con;
	context_init(&con);
	load_functions(con.map);
	if (argc==1){
        if (strcmp(argv[0], "exit") == 0) exit(0);
    }
	if (argc==2){
        if (strcmp(argv[1],"?")==0) fsh_info(NULL);
    }

	while (true){
		char * prompt = def_prompt();
		if(prompt == NULL) break;
		char * line = readline(prompt);
		free(prompt);
		if (line == NULL) break;
		if (strlen(line) > 0) {
			add_history(line);
			if (strcmp(line, "exit") == 0) break;
			if (strcmp(line, "?")==0) fsh_info(NULL);
			else
			parse_input_line(line, &con);
		}
		free(line);
	}
	context_dispose(&con);
    return 0;
}