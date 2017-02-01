#ifndef LOG_SERVICE_H
#define LOG_SERVICE_H

#include <stdlib.h>
#include <string.h>
#include "bool.h"

enum log_type {
	ACCESSLOG,
	ERRORLOG,
};

struct connect_time_and_ip {
	char *connect_time;
	char *Ip_address;
};

struct accesslog_params {
	struct connect_time_and_ip time_ip;
	char *domain;
	char *requested_filename;
	int sent_status_code;
	int num_of_bytes_sent;
	char *user_provided_info;
	char *error_msg;
};

void log_request(enum log_type log_level, void *log_data, char *logfile_path);
void log_struct_dispose(enum log_type log_level, void *log_data);

struct accesslog_params * build_log_data(struct connect_time_and_ip time_ip, char *domain, char *requested_filename,
												int sent_status_code, int num_of_bytes_sent, char *user_provided_info);
												
struct accesslog_params * build_error_log(struct connect_time_and_ip time_and_ip_log, char *error_msg);												

#endif
