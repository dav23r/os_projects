#include "worker.h"
#include "response_builder.h"
#include "config_service.h"
#include "cgi_runner.h"
#include "scan_documents_directory.h"
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


void proccess_request(int in_fd, hashset *config)
{
	if (in_fd < 0) return;
	while (true)
	{
		char request[BUFFER_SIZE];
		request[0] = '\0';
		int bytes_recieved;
		if ((bytes_recieved = recv(in_fd, request, BUFFER_SIZE, 0)) <= 0) break;
		char header[BUFFER_SIZE], response[BUFFER_SIZE];
		header[0] = '\0', response[0] = '\0';
		get_header(request, header);
		
		struct header_info parsed_header;
		parsed_header.host = get_header_value(header, "Host");
		char *document_root = get_config_value(parsed_header.host, "documentroot", config);
		char *cgi_bin = get_config_value(parsed_header.host, "cgi-bin", config);
		if ((strcmp(document_root, NO_KEY_VALUE) != 0 && strlen(document_root) > 0) ||
			((strcmp(cgi_bin, NO_KEY_VALUE) != 0 && strlen(cgi_bin) > 0)))
		{
			/* here also we get requested filename and extension if type is 'FILE', or program name and content-info if CGI */
			parsed_header.method = get_request_method_and_type(header, &parsed_header);
			parsed_header.etag = get_header_value(header, "Etag");
			parsed_header.keep_alive = keep_alive(header);
			parsed_header.range = get_header_range(header);
			
			
			if (parsed_header.cgi_or_file == STATIC_FILE || parsed_header.cgi_or_file == DIR)
			{
				if (strcmp(document_root, NO_KEY_VALUE) == 0 || strlen(document_root) == 0)
					add_initial_header(response, "HTTP/1.0 404 Not Found", strlen(response));
				else
				{
					char *file_path = parsed_header.cgi_or_file == DIR ? get_dir_page_path(document_root, parsed_header.requested_objname) : strcat(document_root, parsed_header.requested_objname);
					char *file_new_hash = compute_file_hash(file_path);
					if (strcmp(parsed_header.etag, file_new_hash) == 0)
					{
						add_initial_header(response, "HTTP/1.0 304 Not Modified", strlen(response));
					}
					else
					{
						char count_str[12], content_type[36];
						count_str[0] = '\0', content_type[0] = '\0';
						FILE *fp = fopen(file_path, "r");
						if (fp)
						{
							off_t *offset = (off_t *)&parsed_header.range->start;
							size_t count = parsed_header.range->end - parsed_header.range->start < 0 ? get_file_size(fp) : parsed_header.range->end - parsed_header.range->start;
							sprintf(count_str, "%ld", count);
							detect_content_type(content_type, parsed_header.ext);
							add_header_key_value(response, "Content-Type", content_type);
							add_header_key_value(response, "Content-Length", count_str);
							add_header_key_value(response, "Cache-Control", "max-age=5");
							add_header_key_value(response, "etag", file_new_hash);
							add_initial_header(response, "HTTP/1.0 200 OK", strlen(response));
							int out_fd = fileno(fp);
							ssize_t bytes = sendfile(out_fd, in_fd, offset, count);
							fclose(fp);
						} else add_initial_header(response, "HTTP/1.0 404 Not Found", strlen(response));
					}
					free(file_path);
					free(file_new_hash);
				}
			}
			else // CGI
			{
				if (strcmp(cgi_bin, NO_KEY_VALUE) == 0 || strlen(cgi_bin) == 0)
					add_initial_header(response, "HTTP/1.0 404 Not Found", strlen(response));
				else
				{
					parsed_header.content_type = get_header_value(header, "Content-Type");
					parsed_header.content_length = get_header_value(header, "Content-Length");
					parsed_header.requested_objname = strcat(cgi_bin, parsed_header.requested_objname + 1); // +1 not to have two slashes (//)
					run_cgi_script(&parsed_header, in_fd, config);
				}
			}
		}
		else	// i.e. host is not valid or neither document and cgi-bin directories aren't provided in config file
		{
			add_initial_header(response, "HTTP/1.0 404 Not Found", strlen(response));
		}
		
		if (strlen(response) == 0) // i.e. response is sent from cgi
			send(in_fd, response, strlen(response), 0);
		bool keep_alive = parsed_header.keep_alive;
		header_info_dispose(&parsed_header);
		if (keep_alive) set_keep_alive(in_fd);
		else break;
	}
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

// returns request method, also sets request type in passed header struct
static enum http_method get_request_method_and_type(char *header, struct header_info *header_struct)
{
	enum http_method ret = UNDEFINED;
	bool method_set = false;
	char *initial_line, *token;
	initial_line = strtok (header, "\n");
	token = strtok (initial_line, " ");
	while (token != NULL)
	{
		if (method_set)
		{
			char *ext = get_filename_extension(token);
			
			if (strcmp(ext, "html") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "mp4") == 0)
				header_struct->cgi_or_file = STATIC_FILE;
			else if (strcmp(ext, "/") == 0)
				header_struct->cgi_or_file = DIR;
			else header_struct->cgi_or_file = CGI;
			
			header_struct->requested_objname = token;
			header_struct->ext = ext;
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
	token = strtok(header, " \n");
	while (token != NULL)
	{
		if (!strcmp(token, strcat(key, ":")))
			return strtok(NULL, " \n");
		token = strtok(NULL, " \n");
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
		res->start = 0;
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
	res[0] = '\0', tmp[0] = '\0';
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

static char *get_filename_extension(char *file_path)
{
	const char *ext;
	if (file_path[strlen(file_path)-1] == '/')
		ext = file_path + strlen(file_path)-1;
	else
	{
		const char *last_dot = strrchr(file_path, '.');
		if(!last_dot || last_dot == file_path)
			return NULL;
		else
			ext = last_dot + 1;
	}
    return (char *)ext;
}

static void detect_content_type(char *content_type, const char *ext)
{
	if (strcmp(ext, "html") == 0)
		strcpy(content_type, "text/html");
	else if (strcmp(ext, "mp4") == 0)
		strcpy(content_type, "video/mp4");
	else if (strcmp(ext, "jpg") == 0)
		strcpy(content_type, "image/jpeg");
}

static long int get_file_size(FILE *stream)
{
	if(!stream)  {
		perror ("Error opening file");
		return(-1);
	}
	fseek(stream, 0, SEEK_END);

	long int len = ftell(stream);
}

static char * get_dir_page_path(char *document_root, char *dir_name)
{
	char *doc_root_copy_2 = strdup(document_root);
	if (file_exists(strcat(doc_root_copy_2, "index.html")))
		return doc_root_copy_2;
	free(doc_root_copy_2);
	
	char *doc_root_copy = strdup(document_root);
	char *root_html_path = strcat(doc_root_copy, strcat("document directory pages/", document_root));
	char *full_path_to_file = strcat(root_html_path, dir_name + 1);
	full_path_to_file = strcat(full_path_to_file, ".html");
	
	if (!file_exists(full_path_to_file))
		scan_and_print_directory(full_path_to_file, true);

	return full_path_to_file;
}

static bool file_exists(char *file_path)
{
	return access( file_path, F_OK ) != -1;
}

static void set_keep_alive(int socket_fd)
{
	struct timeval timeout;      
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
}

static void header_info_dispose(struct header_info *header)
{
	free(header->host);
	free(header->etag);
	free(header->ext);
	free(header->requested_objname);
	free(header->content_type);
	free(header->range);
}


