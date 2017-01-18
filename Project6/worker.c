#include "worker.h"


void proccess_request(char *request)
{
	char header[BUFFER_SIZE];
	get_header(request, header);
	
	bool post = is_post(header);
}

static void get_header(char *request, char *header)
{
	int i;
	for (i = 0; i < BUFFER_SIZE - 1; i++)
	{
		if (request[i] == '\n' && request[i+1] == '\n')
		{
			request[i] = '\0';
			break;
		}
		header[i] = request[i];
	}
}

static bool is_post(char *header)
{
	char * token;
	token = strtok (header, " ");
	while (token != NULL)
	{
		if (!strcmp(token, "GET"))
			return false;
		if (!strcmp(token, "POST"))
			return true;
		token = strtok (NULL, " ");
	}
	
	return false;
}
