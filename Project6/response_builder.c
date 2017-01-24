#include "response_builder.h"

// returns result version of response
char * add_header_key_value(char *response, char *key, char *value)
{
	char *key_value = strcat(strcat(key, ":"), strcat(value, "\r\n"));
	return strcat(key_value, response);
}

// returns result version of response
char * add_body(char *response, char *body)
{
	strcat(response, "\r\n");
	return strcat(response, body);
}

