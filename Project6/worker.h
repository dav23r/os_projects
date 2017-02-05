#ifndef WORKER_H
#define WORKER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hashset.h"
#include "bool.h"

#define BUFFER_SIZE 1024

enum http_method {
	UNDEFINED,
	GET,
	POST,
};

enum request_type {
	STATIC_FILE,
	DIR,
	CGI,
};

struct range_info {
	int start;
	int end;
};

struct header_info {
	enum http_method method;
	enum request_type cgi_or_file;
	char *requested_objname;
	char *ext;
	char *host;
	char *etag;
	bool keep_alive;
    /* for cgi */
    char *content_type;
    char *content_length;
    char *path_info;
    char *query_string;
    /***********/
	struct range_info *range;
};


/* Public interface. */
void * work(void *config);

/* Static functions used withing 'worker.c'. */
static void proccess_request(int in_fd, hashset *config);
static void header_info_dispose(struct header_info *header);
static void get_header(char *request, char *header);
static enum http_method get_request_method_and_type(char *header, struct header_info *header_struct);
static char * get_header_value(char *header, char *key);
static bool keep_alive(char *header);
static struct range_info get_range_info(char *header);
static struct range_info * get_header_range(char *header);
static char * compute_file_hash(char *full_path, struct range_info *range);
static char * check_for_query_string(char *token_copy);
static char * get_filename_extension(char *file_path);
static void detect_content_type(char *content_type, const char *ext);
static long int get_file_size(FILE *stream);
static char * get_dir_page_path(char *document_root, char *dir_name);
static bool file_exists(char *file_path);
static void set_keep_alive(int socket_fd);
static char * get_formatted_datetime();


#define INIT_WORKER \
						if (error_msg) {free(error_msg); error_msg = NULL; } \
						char request[BUFFER_SIZE], response[BUFFER_SIZE], header[BUFFER_SIZE]; \
						memset(request,0,BUFFER_SIZE); memset(response,0,BUFFER_SIZE); memset(header,0,BUFFER_SIZE); \
						int bytes_recieved = 0, status_code_sent = 0, sent_content_len = 0; \
						if ((bytes_recieved = recv(in_fd, request, BUFFER_SIZE, 0)) <= 0) break; \
						struct connect_time_and_ip time_and_ip_log; \
						time_and_ip_log.connect_time = get_formatted_datetime(); \
						get_header(request, header); \
						struct header_info parsed_header; \
						parsed_header.host = get_header_value(header, "Host"); \
						time_and_ip_log.Ip_address = get_config_value(parsed_header.host, "ip", config); \
						char *document_root = get_config_value(parsed_header.host, "documentroot", config); \
						char *cgi_bin = get_config_value(parsed_header.host, "cgi-bin", config); \
						bool file_send = false; \
						log_level = ACCESSLOG;

#define SET_HEADERS \
						add_header_key_value(response, "Content-Type", content_type); \
						add_header_key_value(response, "Content-Length", count_str); \
						if (status_code_sent == 206) { \
							add_header_key_value(response, "Accept-Ranges", "bytes"); \
							char content_range_str[49]; \
							sprintf(content_range_str, "bytes %jd-%ld/%d", (intmax_t)offset, offset + sent_content_len-1, file_size); \
							add_header_key_value(response, "Content-Range", content_range_str); \
						} \
						add_header_key_value(response, "Cache-Control", "max-age=5"); \
						add_header_key_value(response, "etag", file_new_hash);

#define FILE_EXIT \
						{ \
					        	error_msg = strdup("cannot open requested static file"); \
					        	log_level = ERRORLOG; \
					            exit(EXIT_FAILURE); \
					    }

#endif

