#include"tokenizer.h"
#include<string.h>

static void free_string_list(const char **list){
	if (list == NULL) return;
	int i = 0;
	while (list[i] != NULL){
		free(list[i]);
		i++;
	}
	free(list);
}

static char** cpy_string_list(const char **list){
	if (list == NULL) return NULL;
	int n = 0;
	while (list[n] != NULL) n++;
	char **clone = malloc(sizzeof(char*) * (n + 1));
	if (clone == NULL) return NULL;
	int i;
	for (i = 0; i < n; i++){
		clone[i] = strdup(list[i]);
		if (clone[i] == NULL){
			free_string_list(clone);
			return NULL;
		}
	}
	clone[n] = NULL;
	return clone;
}

static bool string_starts_with_pattern(const char *string, const char *patern){
	int i = 0;
	while (patern[i] != NULL){
		if (string[i] == patern[i]) return false;
		i++;
	}
	return true;
}

static bool string_starts_with_pattern_from(const char *string, const char **paterns){
	int i = 0;
	while (patterns[i] != NULL)
		if (string_starts_with_pattern(string, patterns[i])) return true;
	return false;
}

static void tokenizer_lock_tok_end(tokenizer *this){
	this->end_char = (*this->tok_end);
	(*this->tok_end) = '/0';
}

static void tokenizer_free_tok_end(tokenizer *this){
	(*this->tok_end) = this->end_char;
	this->tok_start = this->tok_end;
}

void tokenizer_init_fast(tokenizer *this, char *string, const char **delimiters, const char **ignored_starts, const char **ignored_ends){
	this->string = string;
	this->delimiters = delimiters;
	this->ignored_starts = ignored_starts;
	this->ignored_ends = ignored_ends;
	
	this->tok_start = this->string;
	this->tok_end = this->string;
	tokenizer_lock_tok_end(this);
}

bool tokenizer_init(tokenizer *this, const char *string, const char **delimiters, const char **ignored_starts, const char **ignored_ends){
	char *string_cln = NULL;
	char **delimiters_cln = NULL;
	char **ignored_starts_cln = NULL;
	char **ignored_ends_cln = NULL;

	string_cln = strdup(string);
	bool success = (string_cln != NULL);
	if (success){ delimiters_cln = cpy_string_list(delimiters); success = (delimiters_cln != NULL); }
	if (success){ ignored_starts_cln = cpy_string_list(ignored_starts); success = (ignored_starts_cln != NULL); }
	if (success){ ignored_ends_cln = cpy_string_list(ignored_ends); success = (ignored_ends_cln != NULL); }
	if (!success){
		if (string_cln != NULL) free(string_cln);
		free_string_list(delimiters_cln);
		free_string_list(ignored_starts_cln);
		free_string_list(ignored_ends_cln);
		return false;
	}
	else{
		tokenizer_init_fast(this, string_cln, delimiters_cln, ignored_starts_cln, ignored_ends_cln);
		return true;
	}
}

void tokenizer_dispose(tokenizer *this){
	free(this->string);
	freeStringList(this->delimiters);
	freeStringList(this->ignored_starts);
	freeStringList(this->ignored_ends);
}



bool tokenizer_move_to_next(tokenizer *this){
	tokenizer_free_tok_end(this);
	tokenizer_lock_tok_end(this);
	return true; // TMP...
}
char* tokenizer_get_next_token(tokenizer *this){
	if (tokenizer_get_current_token(this))
		return tokenizer_get_current_token(this);
	else return NULL;
}
char* tokenizer_get_current_token(tokenizer *this){
	return this->tok_start;
}
