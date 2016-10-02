#include "context.h"
#include "functions_runner.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void context_init(context *this){
	int num_buckets = 50;

	// Initilize map of functions
	hashset *map = malloc(sizeof(hashset));
	// format: pointer to null terminated function name -> function pointer
	HashSetNew(map, sizeof(char *) + sizeof(func_pointer), num_buckets, StringHash, StringCmp, StringFree);
	this->map = map;

	// Initialize map of aliases
	hashset *aliases = malloc(sizeof(hashset));
	// format: pointer to null terminated alias -> pointer to null terminated program name
	HashSetNew(aliases, sizeof(char *) + sizeof(char *), num_buckets, StringHash, StringCmp, StringPairFree);
	this->aliases = aliases;

	// Initialize map of 
	hashset *variables = malloc(sizeof(hashset));
	// format: pointer to null terminated variable name -> pointer to null terminated string value
	HashSetNew(variables, sizeof(char *) + sizeof(char *), num_buckets, StringHash, StringCmp, StringPairFree);
	this->variables = variables;

	this->no_fork = false;
}


void context_dispose(context *this){
	HashSetDispose(this->variables);
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
void StringPairFree(void *str){
	StringFree(str);     // dealloc key
	StringFree((char **)str + 1); // dealloc value
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

