#include "response_builder.h"
#include "stdlib.h"

// returns result version of response
char * add_header_key_value(char *response, char *key, char *value)
{
	strcat(response, key);
	strcat(response, ": ");
	strcat(response, value);
	strcat(response, "\r\n");
	return response;
}

// add first initial line of header
void add_initial_header(char *response, char *content, int response_current_len)
{
	char tmp[response_current_len + strlen(content) + 1];
	tmp[0] = '\0';
	strcpy(tmp, content);
	strcat(tmp, "\r\n");
	strcat(tmp, response);
	memmove(response, tmp, strlen(tmp));
}

// returns result version of response
char * add_body(char *response, char *body)
{
	strcat(response, "\r\n");
	return strcat(response, body);
}
