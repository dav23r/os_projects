//
// Created by irakli on 9/29/16.
//

#include "fsh_kill.h"
#include <stdio.h>
#include <stdlib.h>

bool fsh_kill_helper(pid_t pid, int signal){

    errno = 0;
    if (kill(pid, signal)<0) {
        error_handler(errno,"killing process");
        return false;
    }

    return true;
}

bool fsh_kill(pos_arguments *args, context *context) {
	if (args == NULL || args->num_args < 2 || args->arguments == NULL)
		return false;
	int pid, signal;
	if (is_valid_integer(args->arguments[0])) {
		pid = atoi(args->arguments[0]);
	}
	else
		return false;

	if (is_valid_integer(args->arguments[1])) {
		signal = atoi(args->arguments[1]);
	}
	else
		return false;

	return fsh_kill_helper(pid, signal);
}