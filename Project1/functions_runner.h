
#ifndef _functions_runner_
#define _functions_runner_

#include "bool.h"
#include "hashset.h"
#include "arguments.h"
#include "context.h"
#include "parser_util.h"
#include "fsh_type.h"
#include <stdio.h>

typedef bool (*func_pointer)(pos_arguments *);

bool execute_command(const token_t *tokens, context *c, bool *error);

bool find_a_flag_for_type(const token_t *command, bool *error);

int get_tokens_len(const token_t *command);

func_pointer searchFn(hashset *map, char *name);



#endif