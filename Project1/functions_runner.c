#include "functions_runner.h"
#include "vector.h"
#include "functions.h"


char * toLowerCase(char *str) {
	char *res = malloc(strlen(str) + 1);
	int i;
	for(i = 0; str[i]; i++)
  		res[i] = tolower(str[i]);

  	return res;
}

bool token_init(token_t *this, const char *string, token_type type) {
	this->string = strdup(string);
	if(this->string == NULL){
		this->type = NO_TYPE;
		return false;
	}
	this->type = type;
}

void token_init_null(token_t *this){
	this->string = NULL;
	this->type = NO_TYPE;
}

token_t token_get_null(){
	token_t token;
	token_init_null(&token);
	return token;
}

void token_dispose(token_t *this){
	if(this->string != NULL) free(this->string);
}

bool token_equals(const token_t *t1, const token_t *t2){
	return ((t1->type == t2->type) && (t1->string == NULL && t2->string == NULL) || ((t1->string != NULL && t2->string != NULL) && (strcmp(t1->string, t2->string) == 0)));
}

bool token_null(const token_t *t) {
	return (t->string == NULL);
}

bool execute_command(const token_t *command, context *c, bool *error) {
	
	if (command == NULL || token_null(&command[0]) || strlen(command[0].string) == 0) return false;
	char *funcname = command[0].string;
	char *lower_case = toLowerCase(funcname);
	if (strcmp("true", lower_case)) return true;
	if (strcmp("false", lower_case)) return false;
	free(lower_case);

	if (!strcmp(funcname, "ulimit")) {
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

