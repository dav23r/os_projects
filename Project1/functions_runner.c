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

  	return res;
}

bool execute_command(const token_t *command, context *c, bool *error) {
	if (command == NULL || token_null(&command[0]) || strlen(command[0].string) == 0) return false;
	char *funcname = command[0].string;
	char *lower_case = toLowerCase(funcname);
	if (!strcmp("true", lower_case)) return true;
	if (!strcmp("false", lower_case)) return false;
	free(lower_case);
	printf("Hello bros\n");
	if (!strcmp(funcname, "ulimit")) {
		printf("1\n");
		if (token_null(&command[1])) // prosta 'ulimit'-ze ras vshvrebit? return;
		if (command[1].string[0] != '-') {
			*error = true;
			return false; // funqciis mere pirvelive flag ar aris
		}
		
		args_and_flags *args = malloc(sizeof(args_and_flags));
		args->num_flags = 0;
		args->command_arguments = NULL;
		vector *flags;
		VectorNew(flags, sizeof(flag *), NULL, 4);
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
			} else {
				if (current->flag_arguments == NULL && current->flag == '0') {
					current->flag_arguments = malloc(sizeof(pos_arguments));
					current->flag_arguments->num_args = 1;
					current->flag_arguments->arguments = &next;
					VectorAppend(flags, current);
					current = malloc(sizeof(flag)); current->flag = '0'; current->flag_arguments = NULL;
				}
			}
		}
		flag *res = malloc(VectorLength(flags) * sizeof(flag *));
		for (i = 0; i < VectorLength(flags); i++) {
			res[i] = *((flag *)VectorNth(flags, i));
		}
		args->flags = res;

		//return fsh_ulimit(args);
		return true;
	} else if (!strcmp(funcname, "type")) {
		printf("2\n");

		bool has_a_flag = find_a_flag_for_type(&command[1], error);
		if (*error) return false;

		pos_arguments *args = malloc(sizeof(pos_arguments));
		int len = get_tokens_len(command);
		char **arguments = malloc(len * sizeof(char *));

		int k;
		for (k = 1; !token_null(&command[k]); k++) {
			arguments[k-1] = command[k].string;
		}

		args->arguments = arguments;
		args->num_args = len - 1;

		//return fsh_type(has_a_flag, args);
		return true;
	} else {
        printf("else-shi var\n");
		func_pointer fn;
		if ((fn = searchFn(c->map, funcname)) != NULL) { // i.e. built in
            printf("ifshic broooo\n");
			pos_arguments *args = malloc(sizeof(pos_arguments));
			int len = get_tokens_len(command);
			char **arguments = malloc(len * sizeof(char *));

			int k;
			for (k = 1; !token_null(&command[k]); k++) {
				arguments[k-1] = command[k].string;
			}

			args->arguments = arguments;
			args->num_args = len - 1;

			func_pointer fn = searchFn(c->map, funcname);

			return fn(args);
		} else { // program call
			printf("noooot found in built ins\n");
			return fsh_nice('n', 0, funcname, NULL);
		}
	}

	return true;
}

bool find_a_flag_for_type(const token_t *command, bool *error) {
	if (command == NULL) return false;

	int i;
	for (i = 0; !token_null(command + 1); i++) {
		token_t curr = command[i];
		if (strlen(curr.string) == 0) continue;

		if (curr.string[0] == '-') {
			if (strlen(curr.string) == 1 || curr.string[1] != 'p') {
				*error = true;
				return false;
			} else if (i == 0) {
				return true;
			} else return false;
		}
	}
	return false;
}

int get_tokens_len(const token_t *command) {
	int k;
	for (k = 0; !token_null(&command[k]); k++){}

	return k+1;
}

func_pointer searchFn(hashset *map, char *name) {

	void *elem = HashSetLookup(map, &name);
    if (!elem) return NULL;
    printf("Hashset lookup passed\n");
	func_pointer fn = *((func_pointer *) ((char *) elem + sizeof(char **)));
    printf("func_pinter passed\n");
	return fn;
}
