#include "context.h"
#include "assert.h"
#include "hashset.h"
#include "string.h"
#include <stdio.h>

int main(void){

	context c;
	context_init(&c);
	hashset *variables = c.variables;

	char *key = strdup("key");
	char *value = strdup("value");
	char *unit[] = {key, value};

	HashSetEnter(variables, unit);
	void *pt = HashSetLookup(variables, &key);

	if (pt == NULL) printf("nulll");
	printf( "%s\n", * ((char **)pt + 1) );

	context_dispose(&c);
	return 0;
}
