//
// Created by irakli on 9/27/16.
//

#ifndef PROJECT1_UTIL_H
#define PROJECT1_UTIL_H

#include "bool.h"
/*receives an error number and displays it accordingly*/
void error_handler(int errnum, char * errmsg);

/*simple pair of c strings*/
typedef struct{
	char *a;
	char *b;
} string_pair;

bool string_pair_init(string_pair *this, const char *a, const char *b);
void string_pair_dispose(string_pair *this);
bool string_pair_cpy_construct(string_pair *this, const string_pair *src);
bool string_pair_cpy(string_pair *dst, const string_pair *src);

/* constants */
const string_pair ESCAPE_SEQUENCES[];
/*returns path of program via passed argument res or returns false*/
bool get_path_of_program(char * program_name, char * res);

// checks if arg contains only digits
bool is_valid_integer(char *arg);
#endif //PROJECT1_UTIL_H
