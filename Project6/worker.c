#include "worker.h"


void proccess_request(char *request)
{
	char header[BUFFER_SIZE];
	get_header(request, header);
	
	struct header_info parsed_header;
	parsed_header.method = is_post(header);
	parsed_header.host = get_header_value(header, "Host");
	parsed_header.etag = get_header_value(header, "Etag");
	parsed_header.keep_alive = keep_alive(header);
}

static void get_header(char *request, char *header)
{
	int i;
	for (i = 0; i < BUFFER_SIZE - 3; i++)
	{
		if (request[i] == '\r' && request[i+1] == '\n' && request[i+2] == '\r' && request[i+3] == '\n')
		{
			request[i] = '\0';
			break;
		}
		header[i] = request[i];
	}
}

static enum http_method is_post(char *header)
{
	char *initial_line, *token;
	initial_line = strtok (header, "\r\n");
	token = strtok (initial_line, " ");
	while (token != NULL)
	{
		if (!strcmp(token, "GET"))
			return GET;
		if (!strcmp(token, "POST"))
			return POST;
		token = strtok (NULL, " ");
	}
	
	return UNDEFINED;
}

static char *get_header_value(char *header, char *key)
{
	char *token;
	token = strtok(header, " \r\n");
	while (token != NULL)
	{
		if (!strcmp(token, strcat(key, ":")))
			return strtok(NULL, " \r\n");
		token = strtok(NULL, " \r\n");
	}
	return NULL;
}

static bool keep_alive(char *header)
{
	char *value = get_header_value(header, "Connection");
	if (!value || strcmp(value, "keep-alive") != 0)
		return false;
	
	return true;
}

static void header_info_despose(struct header_info *header)
{
	free(header->host);
	free(header->etag);
}


