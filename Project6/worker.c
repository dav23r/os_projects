#include "worker.h"
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>


void proccess_request(int in_fd)
{
	if (in_fd < 0) return;
	char request[BUFFER_SIZE], header[BUFFER_SIZE];
	int bytes_recieved = recv(in_fd, request, BUFFER_SIZE, 0);
	get_header(request, header);
	
	struct header_info parsed_header;
	parsed_header.method = get_request_method_and_type(header, &parsed_header);
	parsed_header.host = get_header_value(header, "Host");
	parsed_header.etag = get_header_value(header, "Etag");
	parsed_header.keep_alive = keep_alive(header);
	parsed_header.range = get_header_range(header);
	
	if (parsed_header.cgi_or_file == STATIC_FILE)
	{
		// SEND FILE
	}
	else
	{
		// CGI
	}	
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

// returns request method, also sets request type in passed header struct
static enum http_method get_request_method_and_type(char *header, struct header_info *header_struct)
{
	enum http_method ret = UNDEFINED;
	bool method_set = false;
	char *initial_line, *token;
	initial_line = strtok (header, "\r\n");
	token = strtok (initial_line, " ");
	while (token != NULL)
	{
		if (method_set)
		{
			const char *ext = get_filename_extension(token);
			if (strcmp(ext, "html") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "mp4") == 0)
				header_struct->cgi_or_file = STATIC_FILE;
			else header_struct->cgi_or_file = CGI;
			return ret;
		}
		else
		{
			if (!strcmp(token, "GET"))
			{
				ret = GET;
				method_set = true;
			}
			if (!strcmp(token, "POST"))
			{
				ret = POST;
				method_set = true;
			}
		}
		token = strtok (NULL, " ");
	}
	
	return ret;
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
	
	sprintf(tmp, "%u_%c", (unsigned)attr.st_size, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%ld_%c", (long)attr.st_mtime, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%ld%c", (long)attr.st_atime, '\0');
	strcat(res, tmp);
	
	return strdup(res);
}

static const char *get_filename_extension(char *file_path)
{
    const char *last_dot = strrchr(file_path, '.');
    if(!last_dot || last_dot == file_path)
		return "";
    return last_dot + 1;
}

static void header_info_despose(struct header_info *header)
{
	free(header->host);
	free(header->etag);
}


