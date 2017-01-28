#ifndef WORKER_H
#define WORKER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bool.h"

const int BUFFER_SIZE = 1024;

enum http_method {
	UNDEFINED,
	GET,
	POST,
};

enum request_type {
	STATIC_FILE,
	CGI,
};

struct range_info {
	int start;
	int end;
};

struct header_info {
	enum http_method method;
	enum request_type cgi_or_file;
	char *requested_filename; 	// if request type is 'static file'
	char *ext;					// if request type is 'static file', file extension
	char *host;
	char *etag;
	bool keep_alive;
	struct range_info *range;
};

static void header_info_despose(struct header_info *header);

void proccess_request(int in_fd, char *config);
static void get_header(char *request, char *header);
static enum http_method get_request_method_and_type(char *header, struct header_info *header_struct);
static char *get_header_value(char *header, char *key);
static bool keep_alive(char *header);
static struct range_info get_range_info(char *header);
static struct range_info * get_header_range(char *header);
static char * compute_file_hash(char *full_path);
static const char *get_filename_extension(char *file_path);
static void detect_content_type(char *content_type, char *ext);
static long int get_file_size(FILE *stream);


#endif

