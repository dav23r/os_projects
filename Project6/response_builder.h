#ifndef RESPONSE_BUILDER_H
#define RESPONSE_BUILDER_H

#include <string.h>

char * add_header_key_value(char *response, char *key, char *value);

char * add_body(char *response, char *body);


#endif
