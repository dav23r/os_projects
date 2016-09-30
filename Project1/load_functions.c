
#include "load_functions.h"
#include <stdlib.h>
#include <string.h>


void load_functions(hashset *map) {

	storeFuncIntoMap(map, "fsh_kill",		fsh_kill);
	storeFuncIntoMap(map, "fsh_nice",		fsh_nice);
	storeFuncIntoMap(map, "fsh_cd",			fsh_cd);
	storeFuncIntoMap(map, "fsh_pwd",		fsh_pwd);
	storeFuncIntoMap(map, "fsh_echo_name",	fsh_echo_name);
	storeFuncIntoMap(map, "fsh_echo_string",fsh_echo_string);
	storeFuncIntoMap(map, "fsh_echo_export",fsh_echo_export);
}

void storeFuncIntoMap(hashset *map, char *fname, func_pointer fn) {
	char *carStr = strdup(fname);
	char buffer[sizeof(char *)+sizeof(func_pointer)];
	memcpy(buffer, &carStr, sizeof(char *));
	memcpy(buffer + sizeof(char *), &fn, sizeof(func_pointer));
	HashSetEnter(map, buffer);
}

