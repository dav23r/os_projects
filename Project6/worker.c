#include "worker.h"
#include <assert.h>
#include "response_builder.h"
#include "config_service.h"
#include "cgi_runner.h"
#include "scan_documents_directory.h"
#include "log_service.h"
#include <sys/stat.h>
#include "webserver.h"
#include <time.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NOT_FOUND \
						{ \
							add_initial_header(response, "HTTP/1.1 404 Not Found", strlen(response)); \
							status_code_sent = 404; \
							add_body(response, "<html><head></head><body><p>404 - NOT FOUND</body></html>"); \
						}

#define RUN_CGI \
						{ \
							parsed_header.content_type = get_header_value(header, "Content-Type"); \
							parsed_header.content_length = get_header_value(header, "Content-Length"); \
							parsed_header.path_info = strdup(parsed_header.requested_objname); \
							parsed_header.requested_objname = strcat(cgi_bin, parsed_header.requested_objname + 1); \
							run_cgi_script(&parsed_header, in_fd, get_config_block(parsed_header.host, config)); \
						}

#define LOG \
						struct accesslog_params *log; \
						if (!error_msg) log = build_log_data(time_and_ip_log, parsed_header.host, parsed_header.requested_objname, status_code_sent, sent_content_len, get_header_value(header, "User-Agent")); \
						else log = build_error_log(time_and_ip_log, error_msg); \
						log_request(log_level, log, get_config_value(parsed_header.host, "log", config)); \
						log_struct_dispose(log_level, log); \
						bool keep_alive = parsed_header.keep_alive; \
						if (keep_alive) set_keep_alive(in_fd); \
						else break;


// this is worker thread, that retrieves client fd from epoll and calls proccess_request() function
void * work(void *config)
{
	// A single event we use as scratch space
	struct epoll_event *event;
	//ertze meti arasodes gvinda rom amovighot, ert threads erti descriptor
	event = (struct epoll_event *)calloc(1, sizeof(struct epoll_event));
	assert(event);

	while (true) {
		//daucdis sanam ar mova rame
		printf("epoll waiting...\n");
		epoll_wait(epoll_fd, event, 1, -1);
		Data *dat = (Data *)event[0].data.ptr;

		if(event[0].events & EPOLLIN) {
			proccess_request(dat->fd, (hashset *)config);
		}
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, dat->fd, event);
		close(dat->fd);
	}
	return NULL;
}

char *error_msg; enum log_type log_level;
// gets request from in_fd, parses, detects what's what client wants, builds response, sends files and logs
static void proccess_request(int in_fd, hashset *config)
{
	if (in_fd < 0) return;
	while (true)
	{
		INIT_WORKER
		if ((strcmp(document_root, NO_KEY_VALUE) != 0 && strlen(document_root) > 0) || ((strcmp(cgi_bin, NO_KEY_VALUE) != 0 && strlen(cgi_bin) > 0)))
		{/* here also we get requested filename and extension if type is 'FILE', or program name and content-info if CGI */
			parsed_header.method = get_request_method_and_type(header, &parsed_header);
			parsed_header.etag = get_header_value(header, "If-None-Match");
			parsed_header.keep_alive = keep_alive(header);
			parsed_header.range = get_header_range(header);
			if (parsed_header.cgi_or_file == STATIC_FILE || parsed_header.cgi_or_file == DIR) {
				if (strcmp(document_root, NO_KEY_VALUE) == 0 || strlen(document_root) == 0) NOT_FOUND
				else {
					char *file_path = parsed_header.cgi_or_file == DIR ? get_dir_page_path(document_root, parsed_header.requested_objname) : strcat(document_root, parsed_header.requested_objname + 1);
					char *file_new_hash = compute_file_hash(file_path, parsed_header.range);
					if (parsed_header.etag && strcmp(parsed_header.etag, file_new_hash) == 0){
						add_initial_header(response, "HTTP/1.1 304 Not Modified", strlen(response));
						status_code_sent = 304;
					} else {
						char count_str[12], content_type[36];
						memset(count_str,0,12); memset(content_type,0,36);
						count_str[0] = '\0', content_type[0] = '\0';
						FILE *fp = fopen(file_path, "r");
						if (fp) {
							off_t offset = parsed_header.range->start;
							int file_size = get_file_size(fp);
							status_code_sent = 206;
							if(parsed_header.range->start >= 0 && parsed_header.range->end >= 0) sent_content_len = parsed_header.range->end - parsed_header.range->start + 1;
							else if (parsed_header.range->end < 0 && parsed_header.range->start >= 0) sent_content_len = file_size - parsed_header.range->start;
							else if (parsed_header.range->start < 0 && parsed_header.range->end >= 0) { offset = 0; sent_content_len = parsed_header.range->end + 1; }
							else { offset = 0; sent_content_len = file_size; status_code_sent = 200; }
							sprintf(count_str, "%d", sent_content_len);
							detect_content_type(content_type, parsed_header.ext);
							SET_HEADERS
							if (status_code_sent == 200) add_initial_header(response, "HTTP/1.1 200 OK", strlen(response));
							else add_initial_header(response, "HTTP/1.1 206 Partial Content", strlen(response));
							fclose(fp);
							int out_fd = open(file_path, O_RDONLY), answ = 0;
					        if (out_fd == -1) FILE_EXIT
					        strcat(response, "\n");
							send(in_fd, response, strlen(response), 0);
					        while (true) { answ = sendfile(in_fd, out_fd, &offset, BUFFER_SIZE); asm volatile(""); if (answ <= 0) break; }
							file_send = true;
							close(out_fd);
						} else NOT_FOUND
					}
				}
			} else { // CGI
				if (strcmp(cgi_bin, NO_KEY_VALUE) == 0 || strlen(cgi_bin) == 0) NOT_FOUND
				else RUN_CGI
			}
		} else NOT_FOUND
		if (strlen(response) != 0 && !file_send) send(in_fd, response, strlen(response), 0);
		LOG
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
	header = strdup(header);
	enum http_method ret = UNDEFINED;
	bool method_set = false;
	char *initial_line, *token;
	char *tokenizer_outer = strdup(header); char *tokenizer_outer_old = tokenizer_outer;

	initial_line = strtok_r (tokenizer_outer, "\n", &tokenizer_outer);

	char *tokenizer_inner = strdup(initial_line);
	char *tokenizer_inner_old = tokenizer_inner;
	token = strtok_r (tokenizer_inner, " ", &tokenizer_inner);
	while (token != NULL)
	{
		if (method_set)
		{
			char *token_copy = strdup(token);
			header_struct->query_string = check_for_query_string(token_copy);
			char *ext = get_filename_extension(token_copy);

			if (strcmp(ext, "html") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "mp4") == 0)
				header_struct->cgi_or_file = STATIC_FILE;
			else if (strcmp(ext, "/") == 0)
				header_struct->cgi_or_file = DIR;
			else header_struct->cgi_or_file = CGI;

			header_struct->requested_objname = token_copy;
			header_struct->ext = ext;

			free (tokenizer_outer_old);
			free (tokenizer_inner_old);

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
		token = strtok_r (tokenizer_inner, " ", &tokenizer_inner);
	}

	free (tokenizer_outer_old);
	free (tokenizer_inner_old);
	return ret;
}

static char *get_header_value(char *header, char *key)
{

	char *token, tmp[strlen(key)+2], header_copy[strlen(header)+1];
	tmp[0] = '\0'; header_copy[0] = '\0';
	strcpy(tmp, key);
	strcat(tmp, ":");
	strcpy(header_copy, header);
	char *tokenizer = strdup(header_copy);
	char *tokenizer_old = tokenizer;
	token = strtok_r(tokenizer, " \n", &tokenizer);
	while (token != NULL)
	{
		if (!strcmp(token, tmp)){
			char *res = strtok_r(tokenizer, "\n", &tokenizer);
			res[strlen(res)-1] = '\0';
			return strdup(res);
		}
		token = strtok_r(tokenizer, " \n", &tokenizer);
	}
	free (tokenizer_old);
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
	char *value = get_header_value(header, "Range");

	// full content requested
	if (!value || strlen(value) == 0)
	{
		res->start = -1;
		res->end = -1;
	}
	else
	{
		char *token;
		char *tokenizer = strdup(value);
		char *tokenizer_old;
		token = strtok_r(tokenizer, "bytes= -", &tokenizer);
		if (token && strlen(token) > 0)
		{
			res->start = atoi(token);
			if ((token = strtok_r(tokenizer, "- ", &tokenizer)) != NULL && strlen(token) > 0)
				res->end = atoi(token);
			else
				res->end = -1;	// i.e. by the end
		}
		free (tokenizer_old);
	}
	return res;
}

static char * compute_file_hash(char *full_path, struct range_info *range)
{
	char res[128], tmp[32];
	res[0] = '\0', tmp[0] = '\0';
	struct stat attr;
	stat(full_path, &attr);

	sprintf(tmp, "%u_%c", (unsigned)attr.st_size, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%ld_%c", (long)attr.st_mtime, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%ld_%c", (long)attr.st_atime, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%d_%c", range->start, '\0');
	strcat(res, tmp);
	sprintf(tmp, "%d%c", range->end, '\0');
	strcat(res, tmp);

	return strdup(res);
}

static char * check_for_query_string(char *token_copy)
{
	char *question_mark = strrchr(token_copy, '?');
	if (question_mark)
	{
		*question_mark = '\0';
		return strdup(question_mark + 1);
	}
	return NULL;
}

static char *get_filename_extension(char *file_path)
{
	const char *ext;
	if (file_path[strlen(file_path)-1] == '/')
	file_path[strlen(file_path)-1] = '\0';

	const char *last_dot = strrchr(file_path, '.');
	if(!last_dot || last_dot == file_path)
		ext = strdup("/");
	else
		ext = strdup(last_dot + 1);

    return (char *)ext;
}

static void detect_content_type(char *content_type, const char *ext)
{
	if (strcmp(ext, "html") == 0 || strcmp(ext, "/") == 0)
		strcpy(content_type, "text/html");
	else if (strcmp(ext, "mp4") == 0)
		strcpy(content_type, "video/mp4");
	else if (strcmp(ext, "jpg") == 0)
		strcpy(content_type, "image/jpeg");
	else{
		error_msg = strdup(" cannot detect MIME type for given content");
		log_level = ERRORLOG;
	}
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
	char doc_root_copy_2[strlen(document_root)+ strlen(dir_name) + 16];
	doc_root_copy_2[0] = '\0';
	strcpy(doc_root_copy_2, document_root);
	strcat(doc_root_copy_2, dir_name+1);
	char dir[strlen(document_root) + strlen(dir_name)]; strcpy(dir, doc_root_copy_2);
	if (doc_root_copy_2[strlen(doc_root_copy_2)-1] != '/') strcat(doc_root_copy_2, "/");
	strcat(doc_root_copy_2, "index.html");
	if (file_exists(doc_root_copy_2))
		return strdup(doc_root_copy_2);

	char doc_root_copy[strlen(document_root) + 64];
	doc_root_copy[0] = '\0';
	strcat(doc_root_copy, "document_directory_pages");
	strcat(doc_root_copy, replace(dir));
	strcat(doc_root_copy, ".html");

	if (!file_exists(doc_root_copy))
		scan_and_print_directory(dir, document_root, true);
	return strdup(doc_root_copy);
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

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    	error_msg = strdup(" cannot keep socket alive");
    	log_level = ERRORLOG;
    }
}

static char * get_formatted_datetime()
{
	time_t rawtime;
	struct tm *timeinfo;

	time (&rawtime);
	timeinfo = localtime(&rawtime);
	return strdup(asctime(timeinfo));
}

static void header_info_dispose(struct header_info *header)
{
	if (!header) return;

	if (header->host) free(header->host);
	if (header->etag) free(header->etag);
	if (header->requested_objname) free(header->requested_objname);
	if (header->ext) free(header->ext);
	if (header->content_type) free(header->content_type);
	if (header->range) free(header->range);
	if (header->content_length) free(header->content_length);
	if (header->path_info) free(header->path_info);
	if (header->query_string) free(header->query_string);
}
