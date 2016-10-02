#include "functions_runner.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include "functions.h"


char * toLowerCase(char *str) {
	char *res = malloc(strlen(str) + 1);
	int i;
	for(i = 0; str[i]; i++)
  		res[i] = tolower(str[i]);
	res[i] = '\0';
  	return res;
}

bool is_assign(char *str) {
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '=') return true;
	}

	return false;
}

char * get_alias_name(char *token, int *index) {
	char buffer[128];
	int i;
	for (i = 0; i < strlen(token); i++) {
		if (token[i] == '=') {
			*index = i+1;
			buffer[i] = '\0';
			break;
		}
		if (token[i] == ' ') return NULL;
		buffer[i] = token[i];
	}

	return strdup(buffer);
}

char * get_alias_value(char *token, int i) {
	char buffer[128];
	int k = 0;
	
	for (; i < strlen(token); i++) {
		if (token[i] == ' ') {
			return NULL;
		}
		buffer[k++] = token[i];
	}
	buffer[k] = '\0';

	return strdup(buffer);
}

bool contains_io_redir(const token_t *command, bool *operand_result, context *c) {

	const char *operand, *filename;
	const token_t *left;
	int i, k;
	token_t *pointer;
	for (i = 0; !token_null(&command[i]); i++) {
		pointer = (token_t *) &command[i]; // cast for non-static = static
	}

	bool operand_found = false;
	for (; pointer != &command[0]; pointer--) {
		for (k = 0; IO_REDIRECT_OPERATORS[k]; k++) {
			if (strcmp(pointer->string, IO_REDIRECT_OPERATORS[k]) == 0) {
				operand = pointer->string;
				operand_found = true;
				break;
			}
		}
		if (operand_found) break;
	}
	if (!operand_found) return false;

	pointer->string = NULL;
	left = command;
	filename = (pointer + 1)->string;

	if (!filename) {
		printf("syntax error in using operand\n");
		pointer->string = (char *) operand;
		return false;
	}

	*operand_result = io_redirect(left, operand, filename, c);
	pointer->string = (char *) operand;

	return true;
}

bool execute_command(const token_t *command, context *c, bool *error) {
	/*printf("%s\n", command[0].string);
	printf("%s\n", command[1].string);
	printf("%s\n", command[2].string);*/
	if (command == NULL || token_null(&command[0]) || strlen(command[0].string) == 0) return false;
	char *funcname = command[0].string;
	char *lower_case = toLowerCase(funcname);
	if (!strcmp("true", lower_case)) return true;
	if (!strcmp("false", lower_case)) return false;
	if (!strcmp("exit", lower_case)) {
		if (token_null(&command[1]) || !is_valid_integer(command[1].string)) exit(0);
		exit(atoi(command[1].string));
	}
	free(lower_case);

	bool operand_result;
	if (contains_io_redir(command, &operand_result, c)) {
		return operand_result;
	} else if (is_assign(command[0].string)) {
		int index;
		char *key = get_alias_name(command[0].string, &index);
		if (!key) return false;
		char *value;
		if (index < strlen(command[0].string)) {
			value = get_alias_value(command[0].string, index);
		} else {
			if (token_null(&command[1]) || command[1].type != STRING || command[1].last_char != '=')
				return false;
			value = strdup(command[1].string);
		}
		char *buffer[] = {key, value};
		HashSetEnter(c->variables, buffer);
		
	} else if (!strcmp(funcname, "ulimit")) {
		if (token_null(&command[1])) { // prosta 'ulimit'
			return fsh_ulimit_helper(get_limit, 'f', 0, 'S');
		}
		if (command[1].string[0] != '-') {
			*error = true;
			return false; // funqciis mere pirvelive flag ar aris
		}

		args_and_flags *args = malloc(sizeof(args_and_flags));
		args->num_flags = 0;
		args->command_arguments = NULL;
        int len = get_tokens_len(command);
        flag *flags = malloc(len * sizeof(flag));
		flag *current = malloc(sizeof(flag)); current->flag = '0'; current->flag_arguments = NULL; // spec values for empty flag

		int i;
		for (i = 1; !token_null(&command[i]); i++) {
			char *next = command[i].string;
			if (strlen(next) == 0) continue;
			if (next[0] == '-') {
				if (strlen(next) == 1)  {
					*error = true;
					return false;
				}

				if (current->flag == '0') current->flag = next[1];
                if (token_null(&command[i+1])) flags[i-1] = *current;
                ++args->num_flags;
			} else {
				if (current->flag_arguments == NULL && current->flag != '0') {
					current->flag_arguments = malloc(sizeof(pos_arguments));
					current->flag_arguments->num_args = 1;
					current->flag_arguments->arguments = &next;
                    flags[i-1] = *current;
					current = malloc(sizeof(flag)); current->flag = '0'; current->flag_arguments = NULL;
                    args->num_flags;
				}
			}
		}
		args->flags = flags;

		bool res = fsh_ulimit(args);
		args_and_flags_free(args);
		return res;
	} else if (!strcmp(funcname, "type")) {
		bool has_a_flag = find_a_flag_for_type(command, error);
		if (*error) return false;

		pos_arguments *args = malloc(sizeof(pos_arguments));
		int len = get_tokens_len(command);
		char **arguments = malloc(len * sizeof(char *));

		int k, n = 0;
		for (k = 1; !token_null(&command[k]); k++) {
			if (strlen(command[k].string) == 0 || (strlen(command[k].string) > 1 && command[k].string[0] == '-' && command[k].string[1] == 'a'))
				continue;
			
			arguments[n++] = command[k].string;
		}

		args->arguments = arguments;
		args->num_args = len - 1;

		bool res = fsh_type(has_a_flag, args, c);
		pos_arguments_free(args);
		return res;
	} else if (!strcmp(funcname, "alias")) {
		if (token_null(&command[1])) {
			printf("syntax error in calling 'alias'\n");
			return false;
		}
		int index;
		char *alias = get_alias_name(command[1].string, &index);
		if (!alias) return false;
		char *prog_name;
		if (index < strlen(command[1].string)) {
			prog_name = get_alias_value(command[1].string, index);
		} else {
			if (token_null(&command[2]) || command[2].type != STRING || command[2].last_char != '=')
				return false;
			prog_name = strdup(command[2].string);
		}

		if (!prog_name) return false;
		pos_arguments *args = malloc(sizeof(pos_arguments));
		args->arguments = malloc(2 * sizeof(char *));
		args->arguments[0] = alias;
		args->arguments[1] = prog_name;
		args->num_args = 2;
		bool res = fsh_alias(args, c);
		pos_arguments_free(args);
		return res;
	} else {
		func_pointer fn = searchFn(c->map, funcname);
        pos_arguments *args = malloc(sizeof(pos_arguments));
        int len = get_tokens_len(command) + 1;
        char **arguments = malloc((len+1) * sizeof(char *));
        int k = (fn == NULL || !strcmp(funcname, "nice") ? 0 : 1), n = 0;
        for (; !token_null(&command[k]); k++) {
            arguments[n++] = command[k].string;
        }

        arguments[len-1] = (char *) c;
        args->arguments = arguments;
        args->num_args = len - 1;

        bool res;
        if (fn)
            res = fn(args);
        else
            res = fsh_nice(args);
	
        pos_arguments_free(args);
        return res;
	}

}

bool find_a_flag_for_type(const token_t *command, bool *error) {
	if (command == NULL) return false;
	int i;
	for (i = 0; !token_null(&command[i]); i++) {
		token_t curr = command[i];
		if (strlen(curr.string) == 0) continue;

		if (curr.string[0] == '-') {
			if (strlen(curr.string) == 1 || curr.string[1] != 'a') {
				*error = true;
				return false;
			} else if (curr.string[1] == 'a') {
				return true;
			}
		}
	}
	return false;
}

int get_tokens_len(const token_t *command) {
	int k;
	for (k = 0; !token_null(&command[k]); k++){}
	return k;
}

func_pointer searchFn(hashset *map, char *name) {

	void *elem = HashSetLookup(map, &name);
    if (!elem) return NULL;
	func_pointer fn = *((func_pointer *) ((char *) elem + sizeof(char **)));
	return fn;
}
