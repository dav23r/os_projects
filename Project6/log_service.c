#include <stdio.h>
#include <assert.h>
#include "log_service.h"

struct accesslog_params * build_log_data(struct connect_time_and_ip time_ip, char *domain, char *requested_filename,
					int sent_status_code, int num_of_bytes_sent, char *user_provided_info)
{
	printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzz\n");
	struct accesslog_params *this = (struct accesslog_params *) malloc(sizeof(struct accesslog_params));
	assert(this);
printf("zdzd donsk %s\n", requested_filename);
	this->time_ip = time_ip;
	this->domain = domain;
	this->requested_filename = requested_filename;
	this->sent_status_code = sent_status_code;
	this->num_of_bytes_sent = num_of_bytes_sent;
	this->user_provided_info = user_provided_info;

	return this;
}

struct accesslog_params * build_error_log(struct connect_time_and_ip time_and_ip_log, char *error_msg)
{
	struct accesslog_params *this = (struct accesslog_params *) malloc(sizeof(struct accesslog_params));
	assert(this);

	this->time_ip = time_and_ip_log;
	this->error_msg = error_msg;
	return this;
}

void log_request(enum log_type log_level, void *log_data, char *logfile_path)
{
	if (log_data == NULL) return;

	char log_str[512];
	log_str[0] = '\0';
	strcat(strcat(log_str, ((struct connect_time_and_ip *)log_data)->connect_time), " ");
	strcat(strcat(log_str, ((struct connect_time_and_ip *)log_data)->Ip_address), (log_level == ACCESSLOG ? " " : "\n"));
	if (log_level == ACCESSLOG)
	{
		printf("%s\n", log_str);
		struct accesslog_params *params = ((struct accesslog_params *)log_data);
		char tmp[32];
		strcat(strcat(log_str, params->requested_filename), " ");
		sprintf(tmp, "%d ", params->sent_status_code);
		strcat(log_str, tmp);
		sprintf(tmp, "%d ", params->num_of_bytes_sent);
		strcat(log_str, tmp);
		strcat(log_str, params->user_provided_info);
		strcat(log_str, "\n");
	} else strcat(log_str, ((struct accesslog_params *)log_data)->error_msg);

	FILE *fp = fopen(logfile_path, "a");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	fprintf(fp, "%s\n", log_str);
	fclose(fp);
}

void log_struct_dispose(enum log_type log_level, void *log_data)
{
	if (log_data == NULL) return;

	free(((struct connect_time_and_ip *)log_data)->connect_time);
	free(((struct connect_time_and_ip *)log_data)->Ip_address);
	
	if (log_level == ACCESSLOG) free(((struct accesslog_params *)log_data)->user_provided_info);
	else free(((struct accesslog_params *)log_data)->error_msg);
}
