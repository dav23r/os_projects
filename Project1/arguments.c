#include "arguments.h"

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


