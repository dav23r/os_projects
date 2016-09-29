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
		REPLACEMENT_END };





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




