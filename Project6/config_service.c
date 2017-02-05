#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "config_service.h"

// argument struct for HashsetMao function that retrives vhost according to the given port number
struct vhost_getter_arg {
	char *port;
	char *ret;
};

void freee(void *a)
{
	free(*(char **)a);
}

// returns config value connected to the vhost and key
char* get_config_value(char *vhost_name, char *key, hashset *configs)
{
	struct config *conf = HashSetLookup(configs, vhost_name);
	if (!conf) return strdup(NO_KEY_VALUE);
	return config_get_value(conf, key);
}

// returns config of the vhost
struct config *get_config_block(char *vhost_name, hashset *configs) { return HashSetLookup(configs, vhost_name); }

// reads configs file and stores in memory
void save_config(const char *configfile, hashset *configs)
{
	//char buff[255];
	FILE *fp = fopen(configfile, "r");
	if (fp == NULL)
        exit(EXIT_FAILURE);

	const char delims[4] = " =";
	char *token, *res;
	struct config curr_conf;
	curr_conf.vhost = NULL;
	char buff[256];
	while (fgets(buff, sizeof(buff), fp))
	{
		char *tmp = strdup(buff);
		char *ptr = tmp;
		if (strlen(buff) <= 1) continue;
		token = strtok_r(tmp, delims, &tmp);
		while (token) {
			if (strcmp(token, "vhost") == 0)
			{
				if (curr_conf.vhost != NULL) HashSetEnter(configs, &curr_conf);
				token = strtok_r(tmp, delims, &tmp);
				token[strlen(token)-1] = '\0';
				curr_conf.vhost = strdup(token);
			}
			else
			{
				char *aa = token, *bb = strtok_r(tmp, delims, &tmp);
				bb[strlen(bb)-1] = '\0';
				config_add_value(&curr_conf, strdup(token), strdup(bb));
				token = bb;
			}
			token = strtok_r(tmp, delims, &tmp);
		}
		free (ptr);
	}
	fclose(fp);
	HashSetEnter(configs, &curr_conf);
}

// return all port numbers from configs
vector * get_all_port_numbers(hashset *configs)
{
	vector *v = (vector *) malloc(sizeof(vector));
	assert(v);
	VectorNew(v, sizeof(char **), freee, 4);

	HashSetMap(configs, ports_getter, v);
	return v;
}

// returns vhostname for the post
char *get_vhost(hashset *configs, char *port)
{
	struct vhost_getter_arg arg;
	arg.port = port;
	HashSetMap(configs, vhost_getter, &arg);
	return arg.ret;
}

// adds new key-value for given config block
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

// returns value for given config block's key
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
	VectorAppend((vector *)auxData, &(((struct config *)elemAddr)->port));
}

static void vhost_getter(void *elemAddr, void *auxData)
{
	if (strcmp(((struct config *)elemAddr)->port, ((struct vhost_getter_arg *)auxData)->port) == 0)
		((struct vhost_getter_arg *)auxData)->ret = ((struct config *)elemAddr)->vhost;
}
