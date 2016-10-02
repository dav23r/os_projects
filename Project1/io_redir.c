#include "io_redir.h"
#include "functions_runner.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

static const char IO_REDIRECT_STDIN_STATIC[] = "<";
const char *IO_REDIRECT_STDIN = IO_REDIRECT_STDIN_STATIC;
static const char IO_REDIRECT_STDOUT_STATIC[] = ">";
const char *IO_REDIRECT_STDOUT = IO_REDIRECT_STDOUT_STATIC;
static const char IO_REDIRECT_STDOUT_APPEND_STATIC[] = ">>";
const char *IO_REDIRECT_STDOUT_APPEND = IO_REDIRECT_STDOUT_APPEND_STATIC;
static const char *IO_REDIRECT_OPERATORS_STATIC[] = { IO_REDIRECT_STDIN_STATIC, IO_REDIRECT_STDOUT_STATIC, IO_REDIRECT_STDOUT_APPEND_STATIC, NULL };
const char **IO_REDIRECT_OPERATORS = IO_REDIRECT_OPERATORS_STATIC;


bool io_redirect(const token_t *command, const char *operand, const char *filename, context *c){
	pid_t child_pid = fork();
	if(child_pid == -1){
		printf("Error occurred while trying to fork\n");
		return false;
	}
	else if(child_pid == 0) {
		int file = -1;
		int flags;
		int stdio_desc;
		if(strcmp(operand, IO_REDIRECT_STDIN) == 0){
			flags = O_RDONLY;
			stdio_desc = STDIN_FILENO;
		} else if(strcmp(operand, IO_REDIRECT_STDOUT) == 0){
			flags = O_WRONLY;
			stdio_desc = STDOUT_FILENO;
		} else if(strcmp(operand, IO_REDIRECT_STDOUT_APPEND) == 0){
			flags = O_WRONLY | O_APPEND;
			stdio_desc = STDOUT_FILENO;
		} else{
			printf("Error: Invalid io redirect operand: %s\n", operand);
			exit(-1);
		}
		file = open(filename, flags);
		if(file >= 0) {
			if(dup2(file, stdio_desc) != -1) {
				close(file);
				bool err = false;
				exit(execute_command(command, c, &err) ? 0 : (-1));
			} else exit(-1);
		} else{
			printf("Error: failed to open file: %s\n", filename);
			exit(-1);
		}
	}else return (wait(NULL) == 0);
}

