
#ifndef _functions_runner_
#define _functions_runner_

#include "bool.h"
#include "hashset.h"


typedef bool (*func_pointer)(pos_arguments *);

typedef enum{
	UNKNOWN,
	STRING,
	NO_TYPE
} token_type;

typedef struct{
	char *string;
	token_type type;
} token_t;

bool token_init(token_t *this, const char *string, token_type type);

void token_init_null(token_t *this);

token_t token_get_null();

void token_dispose(token_t *this);

bool token_equals(const token_t *t1, const token_t *t2);

bool token_null(const token_t *t);

bool execute_command(const token_t *tokens, context *c, bool *error);

bool find_a_flag_for_type(const token_t *command, bool *error);

int get_tokens_len(const token_t *command);

func_pointer searchFn(hashset *map, char *name);



#endif