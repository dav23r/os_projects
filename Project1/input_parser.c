#include "input_parser.h"
#include "tokenizer.h"
#include "parser_exceptions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions_runner.h"

/* /////////////////////////// CONSTANTS: /////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static const char STRING_START_0[] = "\"";
static const char STRING_START_1[] = "\'";
static const char SEMICOLON[] = ";";
static const char COMMENT_START[] = "#";

static const char AND_OPERATOR[] = "&&";
static const char OR_OPERATOR[] = "||";
static const char PIPE_OPERATOR[] = "|";

static const string_pair INPUT_PARSER_IGNORED[] = {
		{ (char*)COMMENT_START, NULL },
		{ (char*)STRING_START_0, (char*)STRING_START_0},
		{ (char*)STRING_START_1, (char*)STRING_START_1},
		IGNORED_END };





/* /////////////////////////// PIPELINE: //////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

//#define PIPELINE_TESTING_MODE

#ifdef PIPELINE_TESTING_MODE
static void pipeline_test_scan_print(){
	int i;
	scanf("%d", &i);
	printf("%d\n", i);
}
#endif

static bool execute_pipeline_process(const token_t **pipeline, context *c, bool *error){
	if(pipeline == NULL || (*pipeline) == NULL) return false;
	else if((*(pipeline + 1)) == NULL){
		bool rv = true;
#ifdef PIPELINE_TESTING_MODE
		pipeline_test_scan_print();
#else
		rv = execute_command(*pipeline, c, error);
#endif
		return (rv && (!(*error)));
	}
	int pipe_fd[2];
	if(pipe(pipe_fd) == -1){
		printf("error creating pipe");
		(*error) = true;
		return false;
	}
	pid_t child_pid = fork();
	if(child_pid == -1){
		printf("Error occurred while trying to fork\n");
		(*error) = false;
		return false;
	}
	else if(child_pid == 0){
		close(pipe_fd[1]);
		bool redirection_success = (dup2(pipe_fd[0], STDIN_FILENO) != -1);
		close(pipe_fd[0]);
		if(redirection_success) {
			bool rv = execute_pipeline_process(pipeline + 1, c, error);
			rv &= (!(error));
			exit(rv ? 0 : (-1));
		}
		else{
			error_handler(errno, "redirection");
			exit(-1);
		}
	}
	else{
		close(pipe_fd[0]);
		bool redirection_success = (dup2(pipe_fd[1], STDOUT_FILENO) != -1);
		close(pipe_fd[1]);
		if(redirection_success) {
			bool rv = true;
#ifdef PIPELINE_TESTING_MODE
			pipeline_test_scan_print();
#else
			rv = execute_command(*pipeline, c, error);
			if(*error) rv = false;
#endif
			if (wait(NULL) != 0) {
				(*error) = true;
				rv = false;
			}
			return rv;
		}
		else{
			error_handler(errno, "redirection");
			(*error) = false;
			return false;
		}
	}
	return true;
}

static bool execute_pipeline(const token_t **pipeline, context *c, bool *error){
	pid_t child_pid = fork();
	if(child_pid == -1){
		printf("Error occurred while trying to fork\n");
		(*error) = false;
		return false;
	}
	else if(child_pid == 0)
		exit(execute_pipeline_process(pipeline, c, error) ? 0 : (-1));
	else return (wait(NULL) == 0);
}

#ifdef PIPELINE_TESTING_MODE
static void log_pipeline(const token_t **pipeline){
	printf("Pipeline:\n");
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
	printf("\n");
}
#endif

static void free_pipeline(token_t **pipeline){
	token_t **cursor;
	for(cursor = pipeline; (*cursor) != NULL; cursor++)
		free_command_tokens(*cursor);
	free(pipeline);
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

#ifdef PIPELINE_TESTING_MODE
	log_pipeline((const token_t**)pipeline);
#endif
	bool result;
	if((*pipeline) == NULL){
		(*error) = true;
		result = false;
	}
	else if((*(pipeline + 1)) == NULL)
		result = execute_command(*pipeline, c, error);
	else result = execute_pipeline((const token_t**)pipeline, c, error);

	free_pipeline(pipeline);

	return result;
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




