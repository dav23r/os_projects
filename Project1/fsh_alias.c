#include "fsh_alias.h"
#include "tokenizer.h"
#include "string.h"

bool fsh_alias(pos_arguments *args, context *context){
	int arg_len = args->num_args;
	if (arg_len != 1) return false;
	char *argument = *(args->arguments);
	// Acquire pointer to the set of aliases
	hashset *aliases = context->aliases;
	// Parse the only argument, ensure to be of type var=value
	tokenizer tok;
	const char *delims[] = { "=", DELIMITER_END };
	string_pair empty[] = { STRING_PAIR_END };
	tokenizer_init(&tok, argument, delims, empty, empty);
	char *alias, *prog_name;
	int num_tokens = 0; bool success = true;
	while (tokenizer_move_to_next(&tok)){
		if (num_tokens > 2) {
			success = false;
			printf("Wrong number of arguments, variable=value expected\n");
			break;
		}
		char *cur_token = tokenizer_get_current_token(&tok);
		if (strlen(cur_token) == 0){
			success = false;
			printf("Don't use whitespaces between tokens\n");
			break;
		}
		if (num_tokens == 0) 
			alias = strdup(cur_token);
		else 
			prog_name = strdup(cur_token);
	}
	if (success)
		add_to_aliases(alias, prog_name, aliases);
	tokenizer_dispose(&tok);
	return success;
}

// Put alias -> prog_name pair to set of aliases
void add_to_aliases(char *alias, char *prog_name, hashset *aliases_set){
	char *set_argument[] = {alias, prog_name};
	HashSetEnter(aliases_set, set_argument);
}
