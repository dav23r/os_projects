
#include "load_functions.h"
#include <stdlib.h>
#include <string.h>


void load_functions(hashset *map) {

	storeFuncIntoMap(map, "kill",			fsh_kill);
	storeFuncIntoMap(map, "nice",			fsh_nice);
	storeFuncIntoMap(map, "cd",			fsh_cd);
	storeFuncIntoMap(map, "pwd",			fsh_pwd);
	storeFuncIntoMap(map, "echo",			fsh_echo);
	storeFuncIntoMap(map, "export",			fsh_echo_export);
	storeFuncIntoMap(map, "?",		        fsh_info);
	/*storeFuncIntoMap(map, "unalias",                fsh_unalias);*/

}

void storeFuncIntoMap(hashset *map, char *fname, func_pointer fn) {
	char *carStr = strdup(fname);
	char buffer[sizeof(char *)+sizeof(func_pointer)];
	memcpy(buffer, &carStr, sizeof(char *));
	memcpy(buffer + sizeof(char *), &fn, sizeof(func_pointer));
	HashSetEnter(map, buffer);
}

