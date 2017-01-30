#ifndef LOG_SERVICE_H
#define LOG_SERVICE_H

#include <stdlib.h>
#include <string.h>
#include "bool.h"

enum log_type {
	accesslog,
	errorlog,
};

struct connect_time_and_ip {
	char *connect_time;
	char *Ip_address;
};

struct accesslog_params {
	struct connect_time_and_ip;
	char *domain;
	char *requested_filename;
	int sent_status_code;
	int num_of_bytes_sent;
	char *user_provided_info;
};

void log_request(enum log_type log_level, void *log_data, char *logfile_path);


#endif
