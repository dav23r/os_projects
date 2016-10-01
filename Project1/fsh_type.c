
#include "fsh_type.h"


bool fsh_type(bool has_a_flag, pos_arguments *args, context *cont){
	// Acquire pointer to hashset of built-in functions
	hashset *builtins = cont->map;
	int i;
	for (i = 0; i < args->num_args; i++){
		char *cur_token = args->arguments[i];
		bool is_shell_builtin = false;
		if (HashSetLookup(builtins, &cur_token) != NULL){
			printf ("%s is shell built-in", cur_token);
			is_shell_builtin = true;         
		}
		// Continue to search in programs only if -a flag is on
                // or the token was not identified as shell built-in
		if (has_a_flag || !is_shell_builtin)
			print_locations_of_program(cur_token, !has_a_flag);
	}

	return true;
}

