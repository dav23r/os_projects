
#include "fsh_type.h"


bool fsh_type(bool has_a_flag, pos_arguments *args, context *cont){
	// Acquire pointers to hashset of built-in functions and aliases
	hashset *builtins = cont->map;
	hashset *aliases = cont->aliases;
	
	char **arguments = args->arguments;
	// For each argument, look it up in data structure in following
	// order: aliases, built-ins, program paths.
	bool every_token_resolved = true;
	int i;
	for (i = 0; i < args->num_args; i++){

		char *cur_argument = arguments[i];
		bool is_found = false;

		// At each stage, stop execution if token was previously
		// found and at the same time 'a' flag is off.
		is_found |= look_up_hashset(aliases, cur_argument);
		if (!has_a_flag && is_found) break; 

		is_found |= look_up_hashset(builtins, cur_argument);
		if (!has_a_flag && is_found) break;
			
		is_found |= print_locations_of_program(cur_argument, !has_a_flag);

		// Remember that we failed to find some argument
		if (!is_found) every_token_resolved = false;
	}

	return every_token_resolved;
}


/* Returns boolean designating if cur_tokean is containted in set */
bool look_up_hashset(hashset *set, char *cur_token){
	void *rv_pointer = HashSetLookup(set, &cur_token);
	return (rv_pointer != NULL);
}

