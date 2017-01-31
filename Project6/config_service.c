#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "config_service.h"

char* get_config_value(char *vhost_name, char *key, hashset *configs)
{
	struct config *conf = HashSetLookup(configs, vhost_name);
	if (!conf) return strdup(NO_KEY_VALUE);
	return config_get_value(conf, key);
}

struct config *get_config_block(char *vhost_name, hashset *configs) { return HashSetLookup(configs, vhost_name); }

void save_config(char *configfile, hashset *configs)
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
			if (curr_conf != NULL) HashSetEnter(configs, curr_conf);
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

vector * get_all_port_numbers(hashset *configs)
{
	vector *v = (vector *) malloc(sizeof(vector));
	assert(v);
	VectorNew(v, sizeof(int), NULL, int 4);
	
	HashSetMap(configs, ports_getter, v);
	return v;
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
		return strdup(conf->document_root);
	else if (strcmp(key, "cgi-bin") == 0)
		return strdup(conf->cgi_bin);
	else if (strcmp(key, "ip") == 0)
		return strdup(conf->ip);
	else if (strcmp(key, "port") == 0)
		return strdup(conf->port);
	else if (strcmp(key, "log") == 0)
		return strdup(conf->log);
}

static void ports_getter(void *elemAddr, void *auxData)
{
	VectorAppend((vector *)auxData, &((config *)elemAddr->port));
}



