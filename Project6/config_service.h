#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "hashset.h"

struct config {
	char *vhost;
	char *document_root;
	char *cgi_bin;
	char *ip;
	char *port;
	char *log;
};

const char NO_KEY_VALUE[] = "no_key_value";

void save_config(char *configfile, hashset *map);

char* get_config_value(char *vhost_name, char *key, hashset *map);

static void config_add_value(struct config *conf, char *key, char *value);
static char * config_get_value(struct config *conf, char *key);

#endif