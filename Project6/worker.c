#include "worker.h"
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>


void proccess_request(char *request)
{
	char header[BUFFER_SIZE];
	get_header(request, header);
	
	struct header_info parsed_header;
	parsed_header.method = is_post(header);
	parsed_header.host = get_header_value(header, "Host");
	parsed_header.etag = get_header_value(header, "Etag");
	parsed_header.keep_alive = keep_alive(header);
	parsed_header.range = get_header_range(header);
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

static struct range_info * get_header_range(char *header)
{
	struct range_info *res = (struct range_info *) malloc(sizeof(struct range_info));
	char *value = get_header_value(header, "range");
	
	// full content requested
	if (!value || strlen(value) == 0)
	{
		res->start = -1;
		res->end = -1;
	}
	else
	{
		char *token;
		token = strtok(value, "- ");
		if (token && strlen(token) > 0)
		{
			res->start = atoi(token);
			
			if ((token = strtok(NULL, "- ")) != NULL && strlen(token) > 0)
				res->end = atoi(token);
			else
				res->end = -1;	// i.e. by the end
		}
	}
	return res;
}

static char * compute_file_hash(char *full_path)
{
	char res[128], tmp[12];
	struct stat attr;
	stat(full_path, &attr);
	
	sprintf(tmp, "%u_\0", (unsigned)attr.st_size);
	strcat(res, tmp);
	sprintf(tmp, "%ld_\0", (long)attr.st_mtime);
	strcat(res, tmp);
	sprintf(tmp, "%ld\0", (long)attr.st_atime);
	strcat(res, tmp);
	
	return strdup(res);
}

static void header_info_despose(struct header_info *header)
{
	free(header->host);
	free(header->etag);
}


