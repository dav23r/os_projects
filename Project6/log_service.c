#include "log_service.h"


void log(enum log_type log_level, void *log_data, char *logfile_path)
{
	if (log_data == NULL) return;
	
	char log_str[512];
	strcat(log_str, ((struct connect_time_and_ip *)log_data)->connect_time + " ");
	strcat(log_str, ((struct connect_time_and_ip *)log_data)->Ip_address + (log_level == accesslog ? " " : ""));
	if (log_level == accesslog)
	{
		struct accesslog_params *params = ((struct accesslog_params *)log_data);
		strcat(log_str, params->requested_filename + " ");
		strcat(log_str, params->sent_status_code + " ");
		strcat(log_str, params->num_of_bytes_sent + " ");
		strcat(log_str, params->user_provided_info);
	}
	
	File *fp = fopen(logfile_path, "w+");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	
	fprintf(fp, log_str + "\n");
	fclose(fp);
}