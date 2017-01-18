#include <stdlib.h>
#include <string.h>
#include "bool.h"

const int BUFFER_SIZE = 1024;

struct range_info {
	int start;
	int end;
};

struct header_info {
	bool is_post;
	char *host;
	char *etag;
	bool keep_alive;
	struct range_info range;
};

void proccess_request(char request[BUFFER_SIZE]);
static void get_header(char *request, char *header);
static bool is_post(char *header);