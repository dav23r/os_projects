#include <stdio.h>
#include <stdlib.h>
#include "config_service.h"

char* get_config_value(char *vhost_name, char *key, hashset *map)
{
	struct config *conf = HashSetLookup(map, vhost_name);
	if (!conf) return strdup(NO_KEY_VALUE);
	return config_get_value(conf, key);
}

void save_config(char *configfile, hashset *map)
{
	char buff[255];
	FILE *fp = fopen(configfile, "r");
	if (fp == NULL)
        exit(EXIT_FAILURE);
	
	const char delims[2] = ":";
	char *token, *res;
	struct config *curr_conf = NULL;
	while (1)
	{
		int ret = fscanf(fp,"%[^\n]", buff);
		if (ret == EOF) break;
		if (strlen(buff) <= 1) continue;
		
		token = strtok(buff, delims);
		if (strcmp(token, "vhost") == 0)
		{
			if (curr_conf != NULL) HashSetEnter(map, curr_conf);
			curr_conf = malloc(sizeof(struct config));
			curr_conf->vhost = strdup(strtok(NULL, delims));
		}
		else
		{
			config_add_value(curr_conf, token, strdup(strtok(NULL, delims)));
		}
	}
	fclose(fp);
}

static void config_add_value(struct config *conf, char *key, char *value)
{
	if (strcmp(key, "documentroot") == 0)
		conf->document_root = value;
	else if (strcmp(key, "cgi-bin") == 0)
		conf->cgi_bin = value;
	else if (strcmp(key, "ip") == 0)
		conf->ip = value;
	else if (strcmp(key, "port") == 0)
		conf->port = value;
	else if (strcmp(key, "log") == 0)
		conf->log = value;
}

static char * config_get_value(struct config *conf, char *key)
{
	if (strcmp(key, "documentroot") == 0)
		return conf->document_root;
	else if (strcmp(key, "cgi-bin") == 0)
		return conf->cgi_bin;
	else if (strcmp(key, "ip") == 0)
		return conf->ip;
	else if (strcmp(key, "port") == 0)
		return conf->port;
	else if (strcmp(key, "log") == 0)
		return conf->log;
}



