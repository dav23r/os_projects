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
static const char PIPE_OPERATOR[] = "|";

static const string_pair INPUT_PARSER_IGNORED[] = {
		{ (char*)COMMENT_START, NULL },
		{ (char*)STRING_START_0, (char*)STRING_START_0},
		{ (char*)STRING_START_1, (char*)STRING_START_1},
		REPLACEMENT_END };





/* /////////////////////////// PIPELINE: //////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static void free_command_tokens(char **tokens){
	char **command_cursor;
	for(command_cursor = tokens; (*command_cursor) != NULL; command_cursor++)
		free(*command_cursor);
	free(*tokens);
}

static void free_pipeline(char ***pipeline){
	char ***cursor;
	for(cursor = pipeline; (*cursor) != NULL; cursor++)
		free_command_tokens(*cursor);
	free(pipeline);
}

static  char **tokenize_command(const char *command){
	printf("Command: <%s>\n", command);
	int len = (strlen(command) + 1);
	char **command_tokens = malloc(sizeof(char*) * len);
	if(command_tokens == NULL) return NULL;

	char *buffer = malloc(sizeof(char) * len);
	if(buffer == NULL){ free(command_tokens); return NULL; }

	tokenizer tok;
	if(!tokenizer_init(&tok, command, NULL, NULL, ESCAPE_SEQUENCES)){
		free(command_tokens);
		free(buffer);
		return NULL;
	}



	tokenizer_dispose(&tok);
	free(buffer);
	return command_tokens;
}

static const char *PIPELINE_DELIMITERS[] = { PIPE_OPERATOR, DELIMITER_END };

static char ***form_pipeline(const char *source){
	int source_len = (strlen(source) + 1);

	char ***pipeline = malloc(sizeof(char**) * source_len);
	if(pipeline == NULL) return NULL;

	char *buffer = malloc(sizeof(char) * source_len);
	if(buffer == NULL){ free(pipeline); return NULL; }

	tokenizer tok;
	if(!tokenizer_init(&tok, source, PIPELINE_DELIMITERS, INPUT_PARSER_IGNORED, ESCAPE_SEQUENCES)){
		free(pipeline);
		free(buffer);
		return NULL;
	}

	char ***pipeline_cursor = pipeline;
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

static bool parse_pipeline(const char *command, context *c, bool *error){
	char ***pipeline = (char***)form_pipeline(command);
	if(pipeline == NULL){
		(*error) = true;
		return false;
	}
	printf("Command: <%s>\n", command);
	free_pipeline(pipeline);
	return true;
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
		if(!tokenizer_move_to_next(&tok)){
			if(operator != NULL)
				printf("-Shell syntax warning: operator not followed by anything (%s)\n", operator);
			break;
		}
		tokenizer_load_raw_token(&tok, command);
		operator = tokenizer_get_last_delimiter(&tok);
		bool result = parse_pipeline(command, c, &error);
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
	if (!tokenizer_init(&tok, line, COMMAND_SEQUENCE_BREAKS, INPUT_PARSER_IGNORED, ESCAPE_SEQUENCES)){
		free(command_sequence);
		return false;
	}

	bool success = true;

	while (tokenizer_move_to_next(&tok)){
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




