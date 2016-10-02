#include "parser_util.h"
#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>


bool token_init(token_t *this, const char *string, token_type type, char last_char) {
	this->string = strdup(string);
	if(this->string == NULL){
		token_init_null(this);
		return false;
	}
	this->type = type;
	this->last_char = last_char;
	return true;
}

void token_init_null(token_t *this){
	this->string = NULL;
	this->type = NO_TYPE;
	this->last_char = '\0';
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

void free_command_tokens(token_t *tokens){
	token_t *command_cursor;
	for(command_cursor = tokens; (!token_null(command_cursor)); command_cursor++)
		token_dispose(command_cursor);
	free(tokens);
}

static const char *STRING_START_ENDS_STATIC[] = { "\"", "\'", DELIMITER_END };
const char **STRING_START_ENDS = STRING_START_ENDS_STATIC;
static const char *WHITE_SPACES_STATIC[] = { " ", "\t", "\r", "\n", "\v", "\f", DELIMITER_END };
const char **WHITE_SPACES = WHITE_SPACES_STATIC;
static const char *COMMAND_DELIMITERS_STATIC[] = { " ", "\t", "\r", "\n", "\v", "\f", "\"", "\'", DELIMITER_END };
const char **COMMAND_DELIMITERS = COMMAND_DELIMITERS_STATIC;
static const string_pair COMMAND_COMMENTS_STATIC[] = { { (char*)"#", NULL }, IGNORED_END };
const string_pair *COMMAND_COMMENTS = COMMAND_COMMENTS_STATIC;

static void string_condense(char **buffer, const char *source){
	while(true){
		(**buffer) = (*source);
		if((*source) == '\0') break;
		(*buffer)++;
		source++;
	}
}

bool string_in_list(const char *string, const char **list){
	if(string == NULL ||  list == NULL) return false;
	while((*list) != NULL){
		if(strcmp(string, (*list)) == 0) return true;
		list++;
	}
	return false;
}

bool token_add(token_t **command_tokens, token_t **command_cursor, const char *token_text, token_type tok_type, char last_cursor){
	if (!token_init(*command_cursor, token_text, tok_type, last_cursor)) {
		free_command_tokens(*command_tokens);
		(*command_tokens) = NULL;
		return false;
	} else (*command_cursor)++;
	return true;
}

token_t *tokenize_command(const char *command){
	int len = (strlen(command) + 1);
	token_t *command_tokens = malloc(sizeof(token_t) * len);
	if(command_tokens == NULL) return NULL;

	char *buffer = malloc(sizeof(char) * len);
	if(buffer == NULL){ free(command_tokens); return NULL; }

	tokenizer tok;
	if(!tokenizer_init(&tok, command, COMMAND_DELIMITERS, COMMAND_COMMENTS, ESCAPE_SEQUENCES)){
		free(command_tokens);
		free(buffer);
		return NULL;
	}

	token_t *command_cursor = command_tokens;
	char last_cursor = '\0';
	const char *last_delimiter = NULL;

	while(tokenizer_move_to_next(&tok)){
		const char *delimiter = tokenizer_get_last_delimiter(&tok);
		const char *token = tokenizer_get_current_token(&tok);
		if((*token) != '\0') {
			if (!token_add(&command_tokens, &command_cursor, token, UNKNOWN, last_cursor)) break;
			last_cursor = token[strlen(token) - 1];
		}
		if(string_in_list(delimiter, STRING_START_ENDS)){
			char *buffer_cursor = buffer;
			while(tokenizer_move_to_next(&tok)){
				string_condense(&buffer_cursor, token);
				const char *cur_delimiter = tokenizer_get_last_delimiter(&tok);
				if(cur_delimiter == NULL || strcmp(delimiter, cur_delimiter) == 0) break;
				if(cur_delimiter != NULL) string_condense(&buffer_cursor, cur_delimiter);
			}
			if(!token_add(&command_tokens, &command_cursor, buffer, STRING, last_cursor)) break;
		}
		last_cursor = ((delimiter != NULL) ? delimiter[strlen(delimiter) - 1] : '\0');
	}

	if(command_tokens != NULL) token_init_null(command_cursor);
	tokenizer_dispose(&tok);
	free(buffer);
	return command_tokens;
}







