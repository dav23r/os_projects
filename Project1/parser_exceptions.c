#include "parser_exceptions.h"
#include "parser_util.h"
#include "tokenizer.h"
#include <string.h>
#include <ctype.h>


static bool string_has_prefix(const char *string, const char *prefix){
	while((*prefix) != '\0'){
		if((*prefix) != (*string)) return false;
		prefix++;
		string++;
	}
	return true;
}

#define RETURN_IF_EOF(pointer) if((*pointer) == '\0'){ (*no_params) = true; return NULL; }
static const token_t *catch_flag_params(const char *input, const char *flag, context *c, bool *internal_error, bool *no_params){
	char *buffer = malloc(sizeof(char) * (strlen(input) + 1));
	if(buffer == NULL){
		(*internal_error) = true;
		return NULL;
	}
	return NULL;
}

typedef bool(*exception_checker)(const char *, context *, bool *);

static const char FLAG_MINUS_C[] = "-c";
static bool check_minus_c(const char *input, context *c, bool *internal_error){
	////////////////////
	///////////////////#
	//////////////////##
	/////////////////###
	////////////////####
	///////////////#####
	//////////////######
	/////////////#######
	////////////########
	///////////#########
	//////////##########
	/////////###########
	////////############
	///////#############
	//////##############
	/////###############
	////################
	///#################
	return false;
}

static const exception_checker EXCEPTIONS[] = {
		check_minus_c,
		NULL
};

bool catch_input_exception(const char *input, context *c, bool *internal_error){
	const exception_checker *checker = EXCEPTIONS;
	while((*checker) != NULL){
		bool res = (*checker)(input, c, internal_error);
		if((*internal_error) || res) break;
		checker++;
	}
	return false;
}
