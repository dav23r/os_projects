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

static const string_pair ESCAPE_SEQUENCES[] = {
		{ "\\\a", "\a" },
		{ "\\\b", "\b" },
		{ "\\\f", "\f" },
		{ "\\\n", "\n" },
		{ "\\\r", "\r" },
		{ "\\\t", "\t" },
		{ "\\\v", "\v" },
		{ "\\\\", "\\" },
		{ "\\\'", "\'" },
		{ "\\\"", "\"" },
		{ "\\\?", "\?" },
		{ NULL, NULL } };

static const char *STRING_START_ENDS[] = { STRING_START_0, STRING_START_1, NULL };





/* ////////////////////// SEQUENCE EXTRACTION: ////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static const char *COMMAND_SEQUENCE_BREAKS[] = { SEMICOLON, NULL };
static const char *COMMAND_LIST_DELIMITERS[] = { STRING_START_0, STRING_START_1, SEMICOLON, NULL };
static const string_pair COMMAND_LIST_IGNORED[] = { { (char*)COMMENT_START, NULL }, { NULL, NULL } };

static bool string_in_list(const char *string, const char **list){
	if(string == NULL || list == NULL) return false;
	const char **li;
	for (li = list; (*li) != NULL; li++)
		if(strcmp(string, *li) == 0) return true;
	return false;
}

static bool move_to_delimiter(tokenizer *tok, const char *delimiter){
	while(true){
		if(!tokenizer_move_to_next(tok)) break;
		const char *del = tokenizer_get_last_delimiter(tok);
		if(del == NULL) return false;
		else if(strcmp(del, delimiter) == 0) return true;
	}
}

static bool fetch_sequence(tokenizer *tok, char *buffer){
	bool rv = false;
	char *cursor = buffer;
	const char *start = tokenizer_get_cursor(tok);
	const char *last_delimiter = tokenizer_get_last_delimiter(tok);
	if(last_delimiter != NULL) start += strlen(last_delimiter);
	while (true){
		if(!tokenizer_move_to_next(tok)) break;
		const char *delimiter = tokenizer_get_last_delimiter(tok);
		if(delimiter == NULL) break;
		else if(string_in_list(delimiter, STRING_START_ENDS)) {
			if (!move_to_delimiter(tok, delimiter)) break;
		}
		else if(string_in_list(delimiter, COMMAND_SEQUENCE_BREAKS)){
			rv = true;
			break;
		}
	}
	const char *end = tokenizer_get_cursor(tok);
	while(start != end){
		(*cursor) = (*start);
		cursor++;
		start++;
	}
	(*cursor) = '\0';
	return rv;
}

/* /////////////////////// PUBLIC INTERFACE: //////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

bool parse_input_line(const char *line, context *c){
	char *command_sequence = malloc(sizeof(char) * (strlen(line) + 1));
	if (command_sequence == NULL) return false;
	tokenizer tok;
	if (!tokenizer_init(&tok, line, COMMAND_LIST_DELIMITERS, COMMAND_LIST_IGNORED, ESCAPE_SEQUENCES)){ free(command_sequence); return false; }

	printf("Line: <%s>\n", line);

	while (true){
		const char *start;
		bool br = (!fetch_sequence(&tok, command_sequence));

		printf("Sequence: <%s>\n", command_sequence);

		if(br) break;
	}

	free(command_sequence);
	return true;
}




