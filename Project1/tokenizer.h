
#ifndef __tokenizer__
#define __tokenizer__
#include "util.h"
#include "bool.h"

typedef struct{
	const char *string;
	const char **delimiters;
	const string_pair *ignored;
	const string_pair *replacements;
	const char *cursor, *tok_start;
	char *current_token;
	const char *last_delimiter;
} tokenizer;

#define DELIMITER_END ((char*)NULL)
#define STRING_PAIR_END ((string_pair){ NULL, NULL })
#define IGNORED_END STRING_PAIR_END
#define REPLACEMENT_END STRING_PAIR_END

/** Initializes given tokenizer;
	Notes:
		1. Constructor copies both the string and the delimiters on heap
			(so no worries aboud data being damaged or something);
		2. delimiters, ignored and replacements sizes are determined
			from the number of strings in them before encountering NULL
		3. tokenizer constructed with this function requires calling tokenizer_dispose() function. */
bool tokenizer_init(tokenizer *this, const char *string, const char **delimiters, const string_pair *ignored, const string_pair *replacements);

/** Disposes of the tokenizer */
void tokenizer_dispose(tokenizer *this);


/** Returns true, if the tokenizer can give us one more token
	Note: should be called before retrieving the first token. */
bool tokenizer_move_to_next(tokenizer *this);

/** Moves to the next token and returns it (or NULL if unable)
	Note: View notes for tokenizer_get_next_token(). */
char* tokenizer_get_next_token(tokenizer *this);

/** Returns current token (or empty string, if unable)
	Notes:
		1. Returned token is not copied/reallocated by the tokenizer
			and will change to the next token after calling tokenizer_move_to_next() function;
		2. Freeing the returned value will cause undefined behaviour. */
char* tokenizer_get_current_token(tokenizer *this);

/** Returns true if gets to a token with non-zero length */
bool tokenizer_move_to_next_valid_token(tokenizer *this);

/** Moves to the next non-empty token(if available) and returns it(NULL if unable) */
char* tokenizer_get_next_valid_token(tokenizer *this);

/** Returns last delimiter (NULL if end of line)
	Note: no need for free. */
const char *tokenizer_get_last_delimiter(tokenizer *this);

/** Returns the current unexplored character address */
const char* tokenizer_get_cursor(tokenizer *this);

/** Iterators to the segment of the string from which the token was extracted */
const char* tokenizer_get_raw_iterator_start(tokenizer *this);
const char* tokenizer_get_raw_iterator_end(tokenizer *this);

/** Provided the string sized buffer, the raw token(segment from the original string)
	will be loaded in it; returns true, if not end of line(therefore, token can be free as well);
 	Note: shorter than needed buffer will cause undefined behaviour. */
bool tokenizer_load_raw_token(tokenizer *this, char *buffer);

/** Resets the tokenizer
	Note: tokenizer_get_current_token() will return the same as before,
 		but iterators and tokenizer_load_raw_token() will not work any more. */
void tokenizer_reset(tokenizer *this);

#endif