#include "context.h"
#include "functions_runner.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
	hashset *aliases;
	hashset *map;
} context;

void context_init(context *this){
	// Initilize map of functions
	hashet *map = malloc(sizeof(hashset));
	HashSetNew(map, sizeof(char **) + sizeof(func_pointer), 20, StringHash, StringCmp, StringFree);
	this->map = map;
	// Initialize map of aliases
	hashset *aliases = malloc(sizeof(hashset));
	HashSetNew(aliases, sizeof(char *) + sizeof(char *), 20, StringHash, StringCmp, AliasFree);
	this->aliases = aliases;
}


void context_dispose(context *this){
	HashSetDispose(this->aliases);
	HashSetDispose(this->map);
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

// Dealocates memory of alias -> program name pair
void AliasFree(void *str){
	StringFree(str);     // dealloc alias
	StringFree(str + 1); // dealloc program name
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

