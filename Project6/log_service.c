#include <stdio.h>
#include "log_service.h"


void log_request(enum log_type log_level, void *log_data, char *logfile_path)
{
	if (log_data == NULL) return;
	
	char log_str[512];
	log_str[0] = '\0';
	strcat(strcat(log_str, ((struct connect_time_and_ip *)log_data)->connect_time), " ");
	strcat(strcat(log_str, ((struct connect_time_and_ip *)log_data)->Ip_address), (log_level == accesslog ? " " : ""));
	if (log_level == accesslog)
	{
		struct accesslog_params *params = ((struct accesslog_params *)log_data);
		char tmp[4];
		strcat(strcat(log_str, params->requested_filename), " ");
		strcat(strcat(log_str, itoa(params->sent_status_code, tmp, 10)), " ");
		strcat(strcat(log_str, itoa(params->num_of_bytes_sent, tmp, 10)), " ");
		strcat(log_str, params->user_provided_info);
	}
	
	FILE *fp = fopen(logfile_path, "a");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	
	fprintf(fp, strcat(log_str, "\n"));
	fclose(fp);
}