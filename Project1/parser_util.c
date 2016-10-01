#include "parser_util.h"
#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>


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

void free_command_tokens(token_t *tokens){
	token_t *command_cursor;
	for(command_cursor = tokens; (!token_null(command_cursor)); command_cursor++)
		token_dispose(command_cursor);
	free(tokens);
}


static const char *STRING_START_ENDS[] = { "\"", "\'", DELIMITER_END };
static const char *COMMAND_DELIMITERS[] = { " ", "\t", "\r", "\n", "\v", "\f", "\"", "\'", DELIMITER_END };
static const string_pair COMMAND_COMMENTS[] = { { (char*)"#", NULL }, IGNORED_END };

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

	while(tokenizer_move_to_next(&tok)){
		const char *delimiter = tokenizer_get_last_delimiter(&tok);
		const char *token = tokenizer_get_current_token(&tok);
		const char *token_text = NULL;
		token_type tok_type = NO_TYPE;
		if(string_in_list(delimiter, STRING_START_ENDS)){
			char *buffer_cursor = buffer;
			while(tokenizer_move_to_next(&tok)){
				const char *cur_delimiter = tokenizer_get_last_delimiter(&tok);
				if(cur_delimiter == NULL || strcmp(delimiter, cur_delimiter) == 0) break;
				string_condense(&buffer_cursor, token);
				if(cur_delimiter != NULL) string_condense(&buffer_cursor, cur_delimiter);
			}
			token_text = buffer;
			tok_type = STRING;
		}
		else if((*token) != '\0'){
			token_text = token;
			tok_type = UNKNOWN;
		}
		if(token_text != NULL) {
			if (!token_init(command_cursor, token_text, tok_type)) {
				free_command_tokens(command_tokens);
				command_tokens = NULL;
				break;
			} else command_cursor++;
		}
	}

	if(command_tokens != NULL) token_init_null(command_cursor);
	tokenizer_dispose(&tok);
	free(buffer);
	return command_tokens;
}







