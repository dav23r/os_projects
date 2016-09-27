#include "iterator.h"

void iter_init(iter *this, void *source, int step_size, int length){
	this->source = source;
	this->step_size = step_size;
	this->length = length;
}

bool are_more_tokens(iter *this){
	return (this->length != 0);
}

void *next_token(iter *this){
	void *cur_addr = this->source;
	this->source = (char *) this->source + this->step_size;
	this->length -= 1;
	return cur_addr;
}


