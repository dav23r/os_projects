#ifndef __iterator__
#define __iterator__

#include "bool.h"

/* Provides basic iteration functionality in OO style */
typedef struct{
	void *source;
	int step_size;
	int length;
}iter;

// Constructor arguments (start address, size of step in bytes, logical length)
void iter_init(iter *this, void *source, int step_size, int length);

// Returns boolean value indicating whether more tokens are available
bool are_more_tokens(iter *this);

// Returns address of the next token
void *next_token(iter *this);

#endif
