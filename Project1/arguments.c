#include "arguments.h"
#include <stdlib.h>

/* The function parses line of arguments following some command 
   and return convenient data structure representation of it.
   input       -- line of arguments to be parsed
   num_args    -- how many positional arguments the command expects
   flags       -- string representation of flags available for the command
                  if following format:
                       "flag1flag2...:num_args_for_each,flag3flag4...:num_args_for_each,..."
   sample usage for 'cd' shell built-in:
                  get_opts(line, 1, "PL:0")
*/
args_and_flags* get_opts(char *input, int num_args, char *flags) {
	// Not implemented yet
};

void args_and_flags_free(args_and_flags *arg) {
	if (!arg) return;
	pos_arguments_free(arg->command_arguments);
	flag_free(arg->flags);
	free(arg);
}

void pos_arguments_free(pos_arguments *arg) {
	if (!arg || !arg->arguments) return;
	free(arg->arguments);
	free(arg);
}

void flag_free(flag *arg) {
	if (!arg || !arg->flag_arguments) return;
	pos_arguments_free(arg->flag_arguments);
	free(arg);
}

