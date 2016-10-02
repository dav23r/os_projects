//
// Created by irakli on 9/27/16.
//

#include "util.h"
#include<string.h>
#include "tokenizer.h"

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
static const string_pair ESCAPE_SEQUENCES_STATIC[] = {

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
const string_pair *ESCAPE_SEQUENCES = ESCAPE_SEQUENCES_STATIC;



/* Prints paths where given program with given name resides
   If first_only flag is present, search is terminated on the
   very first occurence of the program */
bool print_locations_of_program(char *program_name, bool first_only){

    const char* s = getenv("PATH");
    // Initialize tokenizer to parse the 'path' env variable
    tokenizer tok;
    const char * delims[] = {":",DELIMITER_END};
    const string_pair empty[] = { STRING_PAIR_END };
    tokenizer_init(&tok, s, delims, empty, empty);

    // Does the file exist at least somewhere?
    bool is_found = false;

    while(tokenizer_move_to_next(&tok)){
        char * cur_path = tokenizer_get_current_token(&tok);
        char * path_with_filename = malloc(strlen(cur_path) + strlen(program_name) + 2);

	// Construct full path of file
        path_with_filename[0] = '\0';
        strcat(path_with_filename, cur_path);
	strcat(path_with_filename, "/\0");
        strcat(path_with_filename, program_name);

        struct stat sb;
        // If current path is a file, print it
        if (stat(path_with_filename, &sb) == 0 && S_ISREG(sb.st_mode)){
            printf("'%s' is a program located at %s\n", program_name, path_with_filename);
            is_found = true;

            if (first_only){
		break;
            }
        } 
    }

    tokenizer_dispose(&tok);
    return is_found;
}

bool is_valid_integer(char *arg) {
	if (strlen(arg) == 0) return false;

	int i = (arg[0] == '-' ? 1 : 0);
	for (; i < strlen(arg); i++) {
		if (arg[i] < '0' || arg[i] > '9' || i > 9)  // also if larger than integer can store
			return false;
	}
	return true;
}
