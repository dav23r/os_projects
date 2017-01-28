#include "response_builder.h"
#include "stdlib.h"

// returns result version of response
char * add_header_key_value(char *response, char *key, char *value)
{
	char *key_value = strcat(strcat(key, ":"), strcat(value, "\r\n"));
	return strcat(key_value, response);
}

// add first initial line of header
void add_initial_header(char *response, char *content, int response_current_len)
{
	char tmp[response_current_len + strlen(content) + 1];
	strcpy(tmp, content);
	strcat(tmp, "\n");
	strcat(tmp, response);
	memmove(response, tmp, strlen(tmp));
}

// returns result version of response
char * add_body(char *response, char *body)
{
	strcat(response, "\r\n");
	return strcat(response, body);
}

