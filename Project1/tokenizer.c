#include"tokenizer.h"
#include<string.h>
#include<stdlib.h>

static const char *DELIMITERS_NONE_STATIC[] = { DELIMITER_END };
const char **DELIMITERS_NONE = DELIMITERS_NONE_STATIC;
static const string_pair STRING_PAIRS_NONE_STATIC[] = { STRING_PAIR_END };
const string_pair *IGNORED_NONE = STRING_PAIRS_NONE_STATIC;
const string_pair *ESCAPE_NONE = STRING_PAIRS_NONE_STATIC;


static void free_list(const void *list, size_t elem_size, bool(*last_check)(const void*), void(*free_fn)(void*)){
	if (list == NULL) return;
	const char* clist = (const char*)list;
	int i = 0;
	while (!last_check(clist + i)){
		free_fn((void*)(clist + i));
		i += elem_size;
	}
	free((void*)list);
}

static void** cpy_list(const void *list, size_t elem_size, bool(*last_check)(const void*), bool(*copy_fn)(void*, const void*), void(*free_fn)(void*)){
	if (list == NULL) return NULL;
	int n = 0;
	while (!last_check((void*)((char*)list + (n * elem_size)))) n++;
	void *clone = malloc(elem_size * (n + 1));
	if (clone == NULL) return NULL;
	int i;
	for (i = 0; i < n*elem_size; i += elem_size){
		if (!copy_fn((void*)((char*)clone + i), (const void*)((char*)list + i))){
			memcpy((void*)((char*)clone + i), (const void*)((char*)list + (n * elem_size)), elem_size);
			free_list((const void*)clone, elem_size, last_check, free_fn);
			return NULL;
		}
	}
	memcpy((void*)((char*)clone + (n * elem_size)), (const void*)((char*)list + (n * elem_size)), elem_size);
	return clone;
}


static void string_free_fn(void *string){ free(*((void**)string)); }
static bool string_last_check(const void *string){ return ((*((char**)string)) == NULL); }
static bool string_copy_fn(void *dst, const void *src){ (*((char**)dst)) = strdup(*((char**)src)); return ((*((char**)dst)) != NULL); }

static void free_string_list(const char **list){ free_list((const void*)list, sizeof(char*), string_last_check, string_free_fn); }
static char** cpy_string_list(const char **list){ return (char**)cpy_list((const void*)list, sizeof(char*), string_last_check, string_copy_fn, string_free_fn); }


static void free_string_pair(void *p){ string_pair_dispose((string_pair*)p); }
static bool string_pair_last_check(const void *p){ return (((const string_pair*)p)->a == NULL && ((const string_pair*)p)->b == NULL); }
static bool string_pair_cpy_fn(void *dst, const void *src){ return string_pair_cpy_construct((string_pair*)dst, (const string_pair*)src); }

static void free_string_pair_list(const string_pair *list){ free_list((const void*)list, sizeof(string_pair), string_pair_last_check, free_string_pair); }
static string_pair* cpy_string_pair_list(const string_pair *list){ return (string_pair*)cpy_list((const void*)list, sizeof(string_pair), string_pair_last_check, string_pair_cpy_fn, free_string_pair); }



static bool string_find_pattern(const char *string, const char *patern){
	if (patern == NULL) return false;
	int i = 0;
	while (patern[i] != '\0'){
		if (string[i] != patern[i]) return 0;
		i++;
	}
	return i;
}

static int list_pattern_search_generic(const char *string, const void *pattern_containers, size_t pattern_container_size, bool(*last_check)(const void*), int(*pattern_check)(const char*, const void*), const void **patern){
	int i = 0;
	while (true){
		const void *pat = ((const void*)((char*)pattern_containers + i));
		if (last_check(pat)) break;
		int result = pattern_check(string, pat);
		if (result > 0){
			if (patern != NULL) (*patern) = pat;
			return result;
		}
		i += pattern_container_size;
	}
	return 0;
}

int string_patern_check(const char *string, const void *patern){ return string_find_pattern(string, *((const char**)patern)); }
static int string_find_pattern_from_strings(const char *string, const char **paterns, const char ***patern){
	return list_pattern_search_generic(string, (const void **)paterns, sizeof(char*), string_last_check, string_patern_check, (const void**)patern);
}

int pair_patern_check(const char *string, const void *patern){ return string_find_pattern(string, ((const string_pair*)patern)->a); }
static int string_find_pattern_from_pairs(const char *string, const string_pair *paterns, const string_pair **patern){
	return list_pattern_search_generic(string, (const void **)paterns, sizeof(string_pair), string_pair_last_check, pair_patern_check, (const void**)patern);
}

bool tokenizer_init(tokenizer *this, const char *string, const char **delimiters, const string_pair *ignored, const string_pair *replacements){
	const char *string_cln = NULL;
	const char **delimiters_cln = NULL;
	const string_pair *ignored_cln = NULL;
	const string_pair *replacements_cln = NULL;
	char *token = NULL;

	string_cln = strdup(string);
	bool success = (string_cln != NULL);
	if (success){ delimiters_cln = (const char**)cpy_string_list(delimiters); success = (delimiters_cln != NULL); }
	if (success){ ignored_cln = (const string_pair*)cpy_string_pair_list(ignored); success = (ignored_cln != NULL); }
	if (success){ replacements_cln = (const string_pair*)cpy_string_pair_list(replacements); success = (replacements_cln != NULL); }
	if (success){
		int i;
		int delta = 0;
		for (i = 0; (!string_pair_last_check((void*)(replacements_cln + i))); i++){
			int len_diff = (strlen(replacements_cln[i].b) - strlen(replacements_cln[i].a));
			if (delta < len_diff) delta = len_diff;
		}
		int size = (strlen(string_cln) * (1 + delta));
		token = malloc((size + 1) * sizeof(char));
		if (token != NULL) for (i = 0; i <= size; i++) token[i] = '\0';
		else success = false;
	}
	if (!success){
		if (string_cln != NULL) free((char*)string_cln);
		free_string_list(delimiters_cln);
		free_string_pair_list(ignored_cln);
		free_string_pair_list(replacements_cln);
		if (token != NULL) free(token);
		return false;
	}
	else{
		this->string = string_cln;
		this->delimiters = delimiters_cln;
		this->ignored = ignored_cln;
		this->replacements = replacements_cln;

		this->current_token = token;
		tokenizer_reset(this);

		return true;
	}
}

void tokenizer_dispose(tokenizer *this){
	free((char*)this->string);
	free_string_list(this->delimiters);
	free_string_pair_list(this->ignored);
	free_string_pair_list(this->replacements);
	free(this->current_token);
}

static bool tokenizer_move_replace_if_needed(tokenizer *this, char **token){
	const string_pair *pair;
	int jump = string_find_pattern_from_pairs(this->cursor, this->replacements, &pair);
	if (jump > 0){
		const char *repl = pair->b;
		while ((*repl) != '\0'){
			(**token) = (*repl);
			(*token)++;
			repl++;
		}
		this->cursor += jump;
		return true;
	}
	else return false;
}

static bool tokenizer_move_break_on_delimiter(tokenizer *this){
	const char **delimiter = NULL;
	string_find_pattern_from_strings(this->cursor, this->delimiters, &delimiter);
	if (delimiter != NULL){
		this->last_delimiter = (*delimiter);
		return true;
	}
	else return false;
}

static bool tokenizer_move_jump_over_ignore_brackets(tokenizer *this){
	const string_pair *pair;
	int ignored = string_find_pattern_from_pairs(this->cursor, this->ignored, &pair);
	if (ignored > 0){
		this->cursor += ignored;
		while (true){
			if ((*this->cursor) == '\0') break;
			int jump = 0;
			int i;
			for (i = 0; (!string_pair_last_check((void*)(this->ignored + i))); i++)
				if (strcmp(pair->a, this->ignored[i].a) == 0) {
					int jmp = string_find_pattern(this->cursor, this->ignored[i].b);
					if (jmp > 0){
						jump = jmp;
						break;
					}
				}
			if (jump > 0){
				this->cursor += jump;
				break;
			}
			else this->cursor++;
		}
		return true;
	}
	else return false;
}

bool tokenizer_move_to_next(tokenizer *this){
	char *token = this->current_token;
	if(this->last_delimiter != NULL) this->cursor += strlen(this->last_delimiter);
	this->last_delimiter = NULL;
	this->tok_start = this->cursor;
	while (true){
		if ((*this->cursor) == '\0') break;
		if (!tokenizer_move_replace_if_needed(this, &token)){
			if (tokenizer_move_break_on_delimiter(this)) break;
			if (!tokenizer_move_jump_over_ignore_brackets(this)){
				(*token) = (*this->cursor);
				token++;
				this->cursor++;
			}
		}
	}
	(*token) = '\0';
	return (token != this->current_token || (*this->cursor) != '\0');
}
char* tokenizer_get_next_token(tokenizer *this){
	if (tokenizer_move_to_next(this))
		return this->current_token;
	else return NULL;
}
char* tokenizer_get_current_token(tokenizer *this){
	return this->current_token;
}

bool tokenizer_move_to_next_valid_token(tokenizer *this){
    while(true){
		if(!tokenizer_move_to_next(this)) return false;
		else if((*tokenizer_get_current_token(this)) != '\0') return true;
	}
}

char* tokenizer_get_next_valid_token(tokenizer *this){
	if (tokenizer_move_to_next_valid_token(this))
		return this->current_token;
	else return NULL;
}

const char *tokenizer_get_last_delimiter(tokenizer *this){
	return this->last_delimiter;
}

const char* tokenizer_get_cursor(tokenizer *this){
	return this->cursor;
}

const char* tokenizer_get_raw_iterator_start(tokenizer *this){
	return this->tok_start;
}
const char* tokenizer_get_raw_iterator_end(tokenizer *this){
	return this->cursor;
}

bool tokenizer_load_raw_token(tokenizer *this, char *buffer){
	const char *cursor;
	for (cursor = this->tok_start; cursor != this->cursor; cursor++){
		(*buffer) = (*cursor);
		buffer++;
	}
	(*buffer) = '\0';
	return ((*(this->tok_start)) == '\0');
}

void tokenizer_reset(tokenizer *this){
	this->cursor = this->string;
	this->tok_start = this->string;
	this->last_delimiter = NULL;
}