
#ifndef __context__
#define __context__

#include "hashset.h"


typedef struct{
	int i; // Temporary; may not compile if empty...
	hashset *map;
} context;

void context_init(context *this);
void context_dispose(context *this);

void context_cpy(const context *from, context *to);

void context_set_variable(context *this, const char *name, const char *value);



#endif
