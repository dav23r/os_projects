
#include "fsh_type.h"

bool fsh_type(bool has_a_flag, pos_arguments *args, context *cont){

	// Acquire pointers to hashset of built-in functions and aliases
	hashset *builtins = cont->map;
	hashset *aliases = cont->aliases;

	const char *builtins_name = "shell built-in";
	const char *aliases_name = "alias";

	char **arguments = args->arguments;
	// For each argument, look it up in data structure in following
	// order: aliases, built-ins, program paths.
	bool every_token_resolved = true;
	int i;
	for (i = 0; i < args->num_args; i++){
		char *cur_argument = arguments[i];
		printf("curr === %s\n", cur_argument);
		bool is_found = false;

		// At each stage, stop execution if token was previously
		// found and at the same time 'a' flag is off.
		is_found = is_found || look_up_hashset(aliases, cur_argument, aliases_name);
		if (!has_a_flag && is_found) continue; 

		is_found = is_found || look_up_hashset(builtins, cur_argument, builtins_name);
		if (!has_a_flag && is_found) continue;
			
		is_found = is_found || print_locations_of_program(cur_argument, !has_a_flag);

		// Remember that we failed to find some argument
		if (!is_found) {
			every_token_resolved = false;
			printf( "Error: '%s' not identified\n", cur_argument );
		}
	}

	return every_token_resolved;
}


/* Returns boolean designating if cur_tokean is containted in set */
bool look_up_hashset(hashset *set, char *cur_token, const char *set_name){
	void *rv_pointer = HashSetLookup(set, &cur_token);
	bool found = (rv_pointer != NULL);
	if (found) 
		printf("'%s' is a %s\n", cur_token, set_name);
	return found;
}

