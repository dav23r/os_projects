#include"tokenizer.h"
#include<string.h>
#include<stdlib.h>

static void free_string_list(const char **list){
	if (list == NULL) return;
	int i = 0;
	while (list[i] != NULL){
		free((char*)list[i]);
		i++;
	}
	free((char**)list);
}

static char** cpy_string_list(const char **list){
	if (list == NULL) return NULL;
	int n = 0;
	while (list[n] != NULL) n++;
	char **clone = malloc(sizeof(char*) * (n + 1));
	if (clone == NULL) return NULL;
	int i;
	for (i = 0; i < n; i++){
		clone[i] = strdup(list[i]);
		if (clone[i] == NULL){
			free_string_list((const char**)clone);
			return NULL;
		}
	}
	clone[n] = NULL;
	return clone;
}

static bool string_find_pattern(const char *string, const char *patern){
	int i = 0;
	while (patern[i] != '\0'){
		if (string[i] != patern[i]) return 0;
		i++;
	}
	return i;
}

static int string_find_pattern_from(const char *string, const char **paterns, const char **patern){
	int i = 0;
	while (paterns[i] != NULL){
		int result = string_find_pattern(string, paterns[i]);
		if (result > 0){
			if (patern != NULL) (*patern) = paterns[i];
			return result;
		}
		i++;
	}
	return 0;
}

bool tokenizer_init(tokenizer *this, const char *string, const char **delimiters, const char **ignored_starts, const char **ignored_ends){
	const char *string_cln = NULL;
	const char **delimiters_cln = NULL;
	const char **ignored_starts_cln = NULL;
	const char **ignored_ends_cln = NULL;
	char *token = NULL;

	string_cln = strdup(string);
	bool success = (string_cln != NULL);
	if (success){ delimiters_cln = (const char**)cpy_string_list(delimiters); success = (delimiters_cln != NULL); }
	if (success){ ignored_starts_cln = (const char**)cpy_string_list(ignored_starts); success = (ignored_starts_cln != NULL); }
	if (success){ ignored_ends_cln = (const char**)cpy_string_list(ignored_ends); success = (ignored_ends_cln != NULL); }
	if (success){ token = strdup(string); success = (token != NULL); }
	if (!success){
		if (string_cln != NULL) free((char*)string_cln);
		free_string_list(delimiters_cln);
		free_string_list(ignored_starts_cln);
		free_string_list(ignored_ends_cln);
		if (token != NULL) free(token);
		return false;
	}
	else{
		this->string = string_cln;
		this->delimiters = delimiters_cln;
		this->ignored_starts = ignored_starts_cln;
		this->ignored_ends = ignored_ends_cln;

		this->cursor = this->string;
		this->current_token = token;
		this->last_delimiter = NULL;
		return true;
	}
}

void tokenizer_dispose(tokenizer *this){
	free((char*)this->string);
	free_string_list(this->delimiters);
	free_string_list(this->ignored_starts);
	free_string_list(this->ignored_ends);
	free(this->current_token);
}

static bool tokenizer_jump_over_patterns_from(tokenizer *this, const char **patterns){
	while (true){
		if ((*this->cursor) == '\0') return false;
		int jump = string_find_pattern_from(this->cursor, patterns, NULL);
		if (jump > 0) this->cursor += jump;
		else return true;
	}
}

bool tokenizer_move_to_next(tokenizer *this){
	char *token = this->current_token;
	this->last_delimiter = NULL;
	tokenizer_jump_over_patterns_from(this, this->delimiters);
	while (true){
		if ((*this->cursor) == '\0') break;
		const char *delimiter = NULL;
		string_find_pattern_from(this->cursor, this->delimiters, &delimiter);
		if (delimiter != NULL){
			this->last_delimiter = delimiter;
			break;
		}
		int ignored = string_find_pattern_from(this->cursor, this->ignored_starts, NULL);
		if (ignored > 0){
			this->cursor += ignored;
			while (true){
				if ((*this->cursor) == '\0') break;
				int jump = string_find_pattern_from(this->cursor, this->ignored_ends, NULL);
				if (jump > 0){
					this->cursor += jump;
					break;
				}
				else this->cursor++;
			}
			tokenizer_jump_over_patterns_from(this, this->delimiters);
		}
		else{
			(*token) = (*this->cursor);
			token++;
			this->cursor++;
		}
	}
	(*token) = '\0';
	return (token != this->current_token);
}
char* tokenizer_get_next_token(tokenizer *this){
	if (tokenizer_get_current_token(this))
		return this->current_token;
	else return NULL;
}
char* tokenizer_get_current_token(tokenizer *this){
	return this->current_token;
}
const char *tokenizer_get_last_delimiter(tokenizer *this){
	return this->last_delimiter;
}
