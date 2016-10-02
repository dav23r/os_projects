#include "fsh_alias.h"
#include "tokenizer.h"
#include "string.h"

bool fsh_alias(pos_arguments *args, context *context){

	int arg_len = args->num_args;
	// [alias, prog_name] expected
	if (arg_len != 2) return false;

	char **arguments = (args->arguments);
	// Ensure both strings are non empty
	int i;
	for (i = 0; i < arg_len; ++i){
		if (strlen(arguments[i]))
			return false;
	}

	char *alias = strdup(arguments[0]), *prog_name = strdup(arguments[1]);

	// Acquire pointer to the set of aliases and add the pair
	hashset *aliases_set = context->aliases;
	add_to_aliases(alias, prog_name, aliases_set);

	return true;
}

// Put alias -> prog_name pair to set of aliases
void add_to_aliases(char *alias, char *prog_name, hashset *aliases_set){
	char *set_argument[] = {alias, prog_name};
	// If alias -> some other value already existed, it will be overwritten
	HashSetEnter(aliases_set, set_argument);
}
