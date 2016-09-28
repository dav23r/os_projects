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
		STRING_PAIR_END };

static const char *STRING_START_ENDS[] = { STRING_START_0, STRING_START_1, DELIMITER_END };





/* /////////////////////// PUBLIC INTERFACE: //////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////// */

static const char *COMMAND_SEQUENCE_BREAKS[] = { SEMICOLON, DELIMITER_END };
static const string_pair COMMAND_LIST_IGNORED[] = {
		{ (char*)COMMENT_START, NULL },
		{ (char*)STRING_START_0, (char*)STRING_START_0},
		{ (char*)STRING_START_1, (char*)STRING_START_1},
		{ NULL, NULL } };

bool parse_input_line(const char *line, context *c){
	char *command_sequence = malloc(sizeof(char) * (strlen(line) + 1));
	if (command_sequence == NULL) return false;
	tokenizer tok;
	if (!tokenizer_init(&tok, line, COMMAND_SEQUENCE_BREAKS, COMMAND_LIST_IGNORED, ESCAPE_SEQUENCES)){ free(command_sequence); return false; }

	while (true){
		const char *start;
		bool br = (!tokenizer_move_to_next(&tok));
		if(br) break;
		tokenizer_load_raw_token(&tok, command_sequence);
		if((*command_sequence) == '\0') continue;

		printf("Sequence: <%s>\n", command_sequence);

		if(br) break;
	}

	free(command_sequence);
	return true;
}




