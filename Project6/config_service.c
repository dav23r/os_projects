#include "config_service.h"

char[] get_config_value(char *vhost_name, char *key)
{
	char buff[255];
	FILE *fp = fopen(CONFIG_PATH, "r");
	if (fp == NULL)
        exit(EXIT_FAILURE);
	
	const char delims[2] = "-";
	char *token;
	int vhost_found = false;
	char[] res;
	while (1)
	{
		int ret = fscanf(fp,"%[^\n]", buff);
		if (ret == EOF) break;
		
		token = strtok(buff, delims);
		if (strcmp(token, "vhost") == 0)
		{
			value = strtok(NULL, delims);
			if (strcmp(value, vhost_name) == 0)
				vhost_found = true;
		}
		else if (vhost_found && strcmp(token, key) == 0)
		{
			res = strtok(buff, delims);
			break;
		}
	}
	fclose(fp);
	if (!vhost_found) return NO_KEY_VALUE;
	return res;
}