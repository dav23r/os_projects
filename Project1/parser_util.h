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

/**
 * Tokenizes the given command string
 * @param command - command string(segment of the user input)
 * @return tokenized command
 */
token_t *tokenize_command(const char *command);

/**
 * Replaces all the variables and aliases with their definitions
 * @param tokens - tokenized command
 * @param c - context
 * @return True, if success
 */
bool replace_variables(token_t *tokens, context *c);

/**
 * Searches for a string in the list that equal to the first parameter
 * @param string - target string
 * @param list - list of the strings (NULL terminated)
 * @return True, if found
 */
bool string_in_list(const char *string, const char **list);

const char **STRING_START_ENDS;
const char **COMMAND_DELIMITERS;
const char **WHITE_SPACES;
const string_pair *COMMAND_COMMENTS;

#endif
