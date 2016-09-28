//
// Created by irakli on 9/27/16.
//

#ifndef PROJECT1_UTIL_C_H
#define PROJECT1_UTIL_C_H
#include "util.h"
#include<string.h>

//error handling from tutorialspoint.com
void error_handler(int errnum, char * errmsg){
    fprintf(stderr, "Value of errno: %d\n", errnum);
    perror("Error printed by perror");
    fprintf(stderr, "Error while executing %s: %s\n",errmsg, strerror( errnum ));
}




bool string_pair_init(string_pair *this, const char *a, const char *b){
	if (a == NULL) this->a = NULL;
	else this->a = strdup(a);
	if (b == NULL) this->b = NULL;
	else this->b = strdup(b);
	if ((this->a == NULL && a != NULL) || (this->b == NULL && b != NULL)){
		string_pair_dispose(this);
		return false;
	}
	else return true;
}
void string_pair_dispose(string_pair *this){
	if (this->a != NULL) free(this->a);
	if (this->b != NULL) free(this->b);
}
bool string_pair_cpy_construct(string_pair *this, const string_pair *src){
	return string_pair_init(this, src->a, src->b);
}
bool string_pair_cpy(string_pair *dst, const string_pair *src){
	string_pair_dispose(dst);
	return string_pair_cpy_construct(dst, src);
}



/* constants */
const string_pair ESCAPE_SEQUENCES[] = {
		{ "\\\a", "\a" },
		{ "\\\b", "\b" },
		{ "\\\f", "\f" },
		{ "\\\n", "\n" },
		{ "\\\r", "\r" },
		{ "\\\t", "\t" },
		{ "\\\v", "\v" },
		{ "\\\\", "\\" },
		{ "\\\'", "\'" },
		{ "\\\"", "\"" },
		{ "\\\?", "\?" },
		STRING_PAIR_END };








#endif //PROJECT1_UTIL_C_H
