#include <stdio.h>
#include <signal.h>
#include "functions.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "bool.h"
#include "input_parser.h"
#include "load_functions.h"
#include "parser_util.h"

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

void analise_argv(int argc, char **argv, context *c){
	if(argc > 2){
		if(strcmp(*(argv + 1), "-c") == 0){
			token_t *tokens = tokenize_command(*(argv + 2));
			if(tokens == NULL) exit(-1);
			bool fail = false;
			bool result = execute_command(tokens, c, &fail);
			free_command_tokens(tokens);
			exit((result && (!fail)) ? 0 : -1);
		}
	}
}

int main(int argc, char **argv) {
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
    if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
        signal(SIGINT, ignore);
	context con;
	context_init(&con);
	load_functions(con.map);
	analise_argv(argc, argv, &con);
	printf("Free Shell started\n");
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