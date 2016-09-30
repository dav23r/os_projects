#include "context.h"
#include "functions_runner.h"
#include <stdlib.h>
#include <string.h>

void context_init(context *this){
	hashset *map;
	HashSetNew(map, sizeof(char **) + sizeof(func_pointer), 20, StringHash, StringCmp, StringFree);
	
	this->map = map;
}

void context_dispose(context *this){
	
	HashSetDispose(this->map);
}

void context_cpy(const context *from, context *to){
	/* NOT YET IMPLEMETED */
}

void context_set_variable(context *this, const char *name, const char *value){
	/* NOT YET IMPLEMETED */
}



static const signed long kHashMultiplier = -1664117991L;
int StringHash(const void *elem, int numBuckets)
{
	char *s = *(char **) elem;
	unsigned long hashcode = 0;
	int i;
	for (i = 0; i < strlen(s); i++)  
    	hashcode = hashcode * kHashMultiplier + tolower(s[i]);  
	return hashcode % numBuckets;                                  
}

/* Dealocates dinamically created C string */
void StringFree(void *str)
{
	free(*(char **)str);
}

/* Compare function for hashset, compares keys as c-strings [uses strcasecmp] */
int StringCmp(const void *elemAddr1, const void *elemAddr2)
{
	return strcasecmp(*(char **) elemAddr1, *(char **) elemAddr2);
}

