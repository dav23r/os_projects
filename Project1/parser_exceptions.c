#include "parser_exceptions.h"
#include "parser_util.h"
#include "tokenizer.h"
#include <string.h>
#include <ctype.h>


static bool string_has_prefix(const char *string, const char *prefix){
	while((*prefix) != '\0'){
		if((*prefix) != (*string)) return false;
		prefix++;
		string++;
	}
	return true;
}

static token_t *catch_flag_params(const char *input, const char *flag, bool *internal_error){
	char *buffer = malloc(sizeof(char) * (strlen(input) + 1));
	if(buffer == NULL){ (*internal_error) = true; return NULL; }
	tokenizer tok;
	if(!tokenizer_init(&tok, input, COMMAND_DELIMITERS, COMMAND_COMMENTS, ESCAPE_SEQUENCES)){
		free(buffer);
		(*internal_error) = true;
		return NULL;
	}
	token_t *tokens = NULL;
	if(tokenizer_move_to_next_valid_token(&tok))
		if(strcmp(tokenizer_get_current_token(&tok), flag) == 0){
			while(string_in_list(tokenizer_get_last_delimiter(&tok), WHITE_SPACES)) {
				tokenizer_move_to_next(&tok);
				if(strlen(tokenizer_get_current_token(&tok)) > 0) break;
			}
			const char *delimiter = tokenizer_get_last_delimiter(&tok);
			const char *token = tokenizer_get_current_token(&tok);
			if(delimiter == NULL && strlen(token) <= 0)
				printf("Error: expected a token after flag: %s\n", flag);
			else{
				if(strlen(token) <= 0){
					const char *cursor = (tokenizer_get_cursor(&tok) + strlen(delimiter));
					const char *end_cursor = cursor;
					while (tokenizer_move_to_next(&tok))
						if(strcmp(tokenizer_get_last_delimiter(&tok), delimiter) == 0)
							end_cursor = tokenizer_get_cursor(&tok);
					if(cursor == end_cursor) printf("Error: expected symbol: %s\n", delimiter);
					else{
						char *buffer_ptr = buffer;
						while (cursor != end_cursor){
							(*buffer_ptr) = (*cursor);
							cursor++;
							buffer_ptr++;
						}
						(*buffer_ptr) = '\0';
						tokens = tokenize_command(buffer);
						if(tokens == NULL) (*internal_error) = true;
						else if(token_null(tokens)){
							printf("Error: expected a valid list of tokens after flag: %s\n", flag);
							free_command_tokens(tokens);
							tokens = NULL;
						}
					}
				}
				else{
					tokens = malloc(sizeof(token_t) * 2);
					if(tokens == NULL) (*internal_error) = true;
					else if(!token_init(tokens, token, UNKNOWN, '\0')) {
						(*internal_error) = true;
						free(tokens);
						tokens = NULL;
					} else token_init_null(tokens + 1);
				}
			}
		}
	tokenizer_dispose(&tok);
	free(buffer);
	return tokens;
}

typedef bool(*exception_checker)(const char *, context *, bool *);

static const char FLAG_MINUS_C[] = "-c";
static bool check_minus_c(const char *input, context *c, bool *internal_error){
	token_t *tokens = catch_flag_params(input, FLAG_MINUS_C, internal_error);
	if(tokens != NULL){
		printf("program: %s; [", tokens->string);
		int i;
		for(i = 1; (!token_null(tokens + i)); i++)
			printf("<%s>", (tokens + i)->string);
		printf("]\n");
		free_command_tokens(tokens);
		return true;
	}
	return false;
}

static const exception_checker EXCEPTIONS[] = {
		check_minus_c,
		NULL
};

bool catch_input_exception(const char *input, context *c, bool *internal_error){
	const exception_checker *checker = EXCEPTIONS;
	while((*checker) != NULL){
		bool res = (*checker)(input, c, internal_error);
		if((*internal_error) || res) return (!(*internal_error));
		checker++;
	}
	return false;
}
