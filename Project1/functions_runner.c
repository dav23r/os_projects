#include "functions_runner.h"


bool token_init(token_t *this, const char *string, token_type type){
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

bool token_null(const token_t *t){
	return (t->string == NULL);
}

bool execute_command(const toke_t *tokens, context *c, bool *error) {
	
	if (command_array_len <= 0 || strlen(command[0]) == 0) return false;
	char *funcname = command[0];

	if (!strcmp(funcname, "ulimit")) {
		if (command_array_len == 1) // prosta 'ulimit'-ze ras vshvrebit? return;
		if (command[1][0] != '-') return false; // funqciis mere pirvelive flag ar aris
		
		args_and_flags *args = malloc(sizeof(args_and_flags));
		args->num_flags = 0;
		args->command_arguments = NULL;
		vector *flags;
		VectorNew(flags, sizeof(flag *), null, 4);
		flag *current = malloc(sizeof(flag)); current->flag = '0'; current->flag_arguments = NULL; // spec values for empty flag
		
		// asawyobia args-and-flags-is struqtura ro gadaeces fsh_ulimit-s pirdapir
		int i;
		for (i = 1; i < command_array_len; i++) {
			char *next = command[i];
			if (strlen(next) == 0) continue;
			if (next[0] == '-') {
				if (strlen(next) == 1) return false;
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

		return fsh_ulimit(args);
	} else if (!strcmp(funcname, "type")) {

	} else {

	}

	return true;
}

