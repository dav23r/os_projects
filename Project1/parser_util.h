#ifndef PARSER_UTIL
#define PARSER_UTIL

#include "bool.h"
#include "util.h"
#include "context.h"

typedef enum{
	UNKNOWN,
	STRING,
	NO_TYPE
} token_type;

typedef struct{
	char *string;
	token_type type;
	char last_char;
} token_t;

bool token_init(token_t *this, const char *string, token_type type, char last_char);

void token_init_null(token_t *this);

token_t token_get_null();

void token_dispose(token_t *this);

bool token_equals(const token_t *t1, const token_t *t2);

bool token_null(const token_t *t);

void free_command_tokens(token_t *tokens);

token_t *tokenize_command(const char *command);

bool replace_variables(token_t *tokens, context *c);

bool string_in_list(const char *string, const char **list);

const char **STRING_START_ENDS;
const char **COMMAND_DELIMITERS;
const char **WHITE_SPACES;
const string_pair *COMMAND_COMMENTS;

#endif
