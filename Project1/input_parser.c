#include "input_parser.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* /////////////////////////// CONSTANTS: /////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static const char STRING_START_0[] = "\"";
static const char STRING_START_1[] = "\'";
static const char SEMICOLON[] = ";";
static const char COMMENT_START[] = "#";

static const char *STRING_START_ENDS[] = { STRING_START_0, STRING_START_1, DELIMITER_END };

static const char AND_OPERATOR[] = "&&";
static const char OR_OPERATOR[] = "||";

static const string_pair INPUT_PARSER_IGNORED[] = {
		{ (char*)COMMENT_START, NULL },
		{ (char*)STRING_START_0, (char*)STRING_START_0},
		{ (char*)STRING_START_1, (char*)STRING_START_1},
		IGNORED_END };





/* ////////////////////////// STRUCTURES: /////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

typedef enum{
	UNKNOWN,
	STRING,
	NO_TYPE
} token_type;

typedef struct{
	char *string;
	token_type type;
} token_t;

static bool token_init(token_t *this, const char *string, token_type type){
	this->string = strdup(string);
	if(this->string == NULL){
		this->type = NO_TYPE;
		return false;
	}
	this->type = type;
}
static void token_init_null(token_t *this){
	this->string = NULL;
	this->type = NO_TYPE;
}
static token_t token_get_null(){
	token_t token;
	token_init_null(&token);
	return token;
}
static void token_dispose(token_t *this){
	if(this->string != NULL) free(this->string);
}
static bool token_equals(const token_t *t1, const token_t *t2){
	return ((t1->type == t2->type) && (t1->string == NULL && t2->string == NULL) || ((t1->string != NULL && t2->string != NULL) && (strcmp(t1->string, t2->string) == 0)));
}
static bool token_null(const token_t *t){
	return (t->string == NULL);
}





/* /////////////////////////// HELPERS: ///////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static void string_condense(char **buffer, const char *source){
	while(true){
		(**buffer) = (*source);
		if((*source) == '\0') break;
		(*buffer)++;
		source++;
	}
}

static bool string_in_list(const char *string, const char **list){
	if(string == NULL ||  list == NULL) return false;
	while((*list) != NULL){
		if(strcmp(string, (*list)) == 0) return true;
		list++;
	}
	return false;
}





/* /////////////////////////// PIPELINE: //////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static bool execute_command(const char **command, int command_array_len, context *c, bool *error) {
	
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


/* /////////////////////////// PIPELINE: //////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static void log_pipeline(const token_t **pipeline){
	const token_t **pipeline_cursor;
	for(pipeline_cursor = pipeline; (*pipeline_cursor) != NULL; pipeline_cursor++){
		printf("Command: [");
		const token_t *command_cursor;
		for(command_cursor = (*pipeline_cursor); (!token_null(command_cursor)); command_cursor++){
			printf("(<%s>:", command_cursor->string);
			switch (command_cursor->type){
				case UNKNOWN:
					printf("UNKNOWN)");
					break;
				case STRING:
					printf("STRING)");
					break;
				case NO_TYPE:
					printf("NO TYPE)");
					break;
				default:
					printf(")");
			}
		}
		printf("]\n");
	}
}

static void free_command_tokens(token_t *tokens){
	token_t *command_cursor;
	for(command_cursor = tokens; (!token_null(command_cursor)); command_cursor++)
		token_dispose(command_cursor);
	free(tokens);
}

static void free_pipeline(token_t **pipeline){
	token_t **cursor;
	for(cursor = pipeline; (*cursor) != NULL; cursor++)
		free_command_tokens(*cursor);
	free(pipeline);
}

static const char *COMMAND_DELIMITERS[] = { " ", "\t", "\r", "\n", "\v", "\f", STRING_START_0, STRING_START_1, DELIMITER_END };
static const string_pair COMMAND_COMMENTS[] = { { (char*)COMMENT_START, NULL }, IGNORED_END };

static token_t *tokenize_command(const char *command){
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

static const char *PIPELINE_DELIMITERS[] = { PIPE_OPERATOR, DELIMITER_END };

static token_t **form_pipeline(const char *source){
	int source_len = (strlen(source) + 1);

	token_t **pipeline = malloc(sizeof(token_t*) * source_len);
	if(pipeline == NULL) return NULL;

	char *buffer = malloc(sizeof(char) * source_len);
	if(buffer == NULL){ free(pipeline); return NULL; }

	tokenizer tok;
	if(!tokenizer_init(&tok, source, PIPELINE_DELIMITERS, INPUT_PARSER_IGNORED, ESCAPE_SEQUENCES)){
		free(pipeline);
		free(buffer);
		return NULL;
	}

	token_t **pipeline_cursor = pipeline;
	while (tokenizer_move_to_next(&tok)) {
		tokenizer_load_raw_token(&tok, buffer);
		(*pipeline_cursor) = tokenize_command(buffer);
		if((*pipeline_cursor) == NULL){
			free_pipeline(pipeline);
			pipeline = NULL;
			break;
		}
		pipeline_cursor++;
	}
	if(pipeline != NULL) (*pipeline_cursor) = NULL;

	tokenizer_dispose(&tok);
	free(buffer);
	return pipeline;
}

static bool parse_pipeline(const char *command, context *c, bool *error) {
	token_t **pipeline = form_pipeline(command);
	if (pipeline == NULL) {
		(*error) = true;
		return false;
	}

	log_pipeline((const token_t**)pipeline);

	free_pipeline(pipeline);
}




/* /////////////////////// COMMAND SEQUENCE: //////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static const char *COMMAND_SEQUENCE_DELIMITERS[] = { AND_OPERATOR, OR_OPERATOR, DELIMITER_END };

static bool parse_command_sequence(const char *sequence, context *c){
	char *command = malloc(sizeof(char) * (strlen(sequence) + 1));
	if (command == NULL) return false;
	tokenizer tok;
	if(!tokenizer_init(&tok, sequence, COMMAND_SEQUENCE_DELIMITERS, INPUT_PARSER_IGNORED, ESCAPE_SEQUENCES)){ free(command); return false; }

	bool error = false;
	const char *operator = NULL;

	while (true){
		bool br = (!tokenizer_move_to_next(&tok));
		if(br){
			if(operator != NULL)
				printf("-Shell syntax warning: operator not followed by anything (%s)\n", operator);
			break;
		}
		tokenizer_load_raw_token(&tok, command);
		operator = tokenizer_get_last_delimiter(&tok);
		bool result = execute_command(command, c, &error);
		if(operator != NULL){
			const char *cursor = tokenizer_get_cursor(&tok) + strlen(operator);
			if ((*cursor) == '&' || (*cursor) == '|'){
				printf("-Shell syntax error:  Unexpected token: %c\n", (*cursor));
				error = true;
				break;
			}
			else {
				if (error) break;
				else if (strcmp(operator, AND_OPERATOR) == 0) {
					if(result) continue;
					else break;
				} else if (strcmp(operator, OR_OPERATOR) == 0) {
					if(result) break;
					else continue;
				}
			}
		}
	}

	free(command);
	tokenizer_dispose(&tok);
	return (!error);
}




/* /////////////////////// PUBLIC INTERFACE: //////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static const char *COMMAND_SEQUENCE_BREAKS[] = { SEMICOLON, DELIMITER_END };

bool parse_input_line(const char *line, context *c){
	char *command_sequence = malloc(sizeof(char) * (strlen(line) + 1));
	if (command_sequence == NULL) return false;
	tokenizer tok;
	if (!tokenizer_init(&tok, line, COMMAND_SEQUENCE_BREAKS, INPUT_PARSER_IGNORED, ESCAPE_SEQUENCES)){ free(command_sequence); return false; }

	bool success = true;

	while (true){
		bool br = (!tokenizer_move_to_next(&tok));
		if(br) break;
		tokenizer_load_raw_token(&tok, command_sequence);
		if((*command_sequence) == '\0') continue;
		else if(!parse_command_sequence(command_sequence, c)){
			success = false;
			break;
		}
	}

	free(command_sequence);
	tokenizer_dispose(&tok);
	return success;
}




