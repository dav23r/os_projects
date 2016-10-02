#include "fsh_unalias.h"

bool fsh_unalias(pos_arguments *args){
	context *c = (context*) (args->arguments[args->num_args - 1]);
	return fsh_unalias_helper(args, c);
}

// Implementation of 'unalias' shell built-in
bool fsh_unalias_helper(pos_arguments *args, context *context){

	if (args->num_args > 2) {
		printf("Too much arguments, only alias name should be given\n");
		return false;
	}

	char *alias = *(args->arguments);
	if (strlen(alias) == 0) {
		printf("Empty alias name given\n");
		return false;
	}	

	printf ("Before lookup\n");
	// Remove alias entry from hashset
	hashset *aliases_set = context->aliases;
	if (HashSetLookup(aliases_set, alias) == NULL){
		printf("Alias not found\n");
		return false;
	}
	printf ("After succ lookup\n");

	HashSetDelete(aliases_set, &alias);
	return true;
}
