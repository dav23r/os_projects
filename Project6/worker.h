#ifndef WORKER_H
#define WORKER_H

#include <stdlib.h>
#include <string.h>
#include "bool.h"

const int BUFFER_SIZE = 1024;

enum http_method {
	UNDEFINED,
	GET,
	POST,
};

struct range_info {
	int start;
	int end;
};

struct header_info {
	enum http_method method;
	char *host;
	char *etag;
	bool keep_alive;
	struct range_info *range;
};

static void header_info_despose(struct header_info *header);

void proccess_request(char *request);
static void get_header(char *request, char *header);
static enum http_method is_post(char *header);
static char *get_header_value(char *header, char *key);
static bool keep_alive(char *header);
static struct range_info get_range_info(char *header);
static struct range_info * get_header_range(char *header);
static char * compute_file_hash(char *full_path);


#endif

