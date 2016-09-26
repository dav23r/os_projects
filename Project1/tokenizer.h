
#ifndef __tokenizer__
#define __tokenizer__

typedef struct{
	char *string;
	const char **delimiters;
	const char **ignored_starts;
	const char **ignored_ends;
	char *tok_start, *tok_end, end_char;
} tokenizer;

/** Initializes tokenizer fast, without copying string or delimiters, 
	temporarily altering input when parsing. 
	Notes:
		1. May be used if and only if the whole text is not needed while working on tokens;
		2. delimiters, ignored_starts and ignored_ends sizes are determined
			from the number of strings in them before encountering NULL
		3. tokenizer_dispose() function SHOULD NOT be called on a tokenizer constructed with this constructor.
*/
void tokenizer_init_fast(tokenizer *this, char *string, const char **delimiters, const char **ignored_starts, const char **ignored_ends);

/** Initializes given tokenizer;
	Notes:
		1. This constructor copies both the string and the delimiters on heap,
			making the structure most reliable for any case, but also a little slower;
		2. delimiters, ignored_starts and ignored_ends sizes are determined
			from the number of strings in them before encountering NULL
		3. tokenizer constructed with this function requires calling tokenizer_dispose() function.
*/
bool tokenizer_init(tokenizer *this, const char *string, const char **delimiters, const char **ignored_starts, const char **ignored_ends);

/** Disposes of the tokenizer */
void tokenizer_dispose(tokenizer *this);


/** Returns true, if the tokenizer can give us one more token
	Note: should be called before retrieving the first token.
*/
bool tokenizer_move_to_next(tokenizer *this);
/** Moves to the next token and returns it (or NULL if unable) */
char* tokenizer_get_next_token(tokenizer *this);
/** Returns current token (or empty string, if unable)
	Note: returned token is not copied/reallocated and altering it may effect internal logic of the tokenizer.
*/
char* tokenizer_get_current_token(tokenizer *this);

#endif