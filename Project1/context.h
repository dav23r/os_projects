
#ifndef __context__
#define __context__

#include "hashset.h"
//#include "functions_runner.h"


typedef struct{
	hashset *aliases;
	hashset *variables;
	hashset *map;
} context;

void context_init(context *this);

void context_dispose(context *this);

void context_cpy(const context *from, context *to);

void context_set_variable(context *this, const char *name, const char *value);

int StringCmp(const void *elemAddr1, const void *elemAddr2);

void StringFree(void *str);

int StringHash(const void *elem, int numBuckets);

void StringPairFree(void *str);

#endif
