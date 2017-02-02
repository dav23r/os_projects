#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "config_service.h"

char* get_config_value(char *vhost_name, char *key, hashset *configs)
{
	printf("count = %d\n", HashSetCount(configs));
	struct config *conf = HashSetLookup(configs, vhost_name);
	printf("conf null? %d\n", conf == NULL);
	if (!conf) return strdup(NO_KEY_VALUE);
	return config_get_value(conf, key);
}

struct config *get_config_block(char *vhost_name, hashset *configs) { return HashSetLookup(configs, vhost_name); }

void save_config(const char *configfile, hashset *configs)
{
printf ("%s\n", configfile);
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
		printf ("buffer - %s\n", buff);
		if (strlen(buff) <= 1) continue;
		token = strtok(buff, delims);
		while (token) {
			printf ("token - %s\n", token);
			if (strcmp(token, "vhost") == 0)
			{
				printf ("%d\n", 11);
				if (curr_conf.vhost != NULL) HashSetEnter(configs, &curr_conf);
				printf ("%d\n", 12);
				//curr_conf = malloc(sizeof(struct config));
				printf ("%d\n", 13);
				token = strtok(NULL, delims);
				token[strlen(token)-1] = '\0';
				printf("aa - %s\n", token);
				curr_conf.vhost = strdup(token);
				printf ("%d\n", 14);
			}
			else
			{
				printf("aaaa\n");
				char *aa = token, *bb = strtok(NULL, delims);
				bb[strlen(bb)-1] = '\0';
				printf("aaa - %s\n", aa);
				printf("bbb - %s\n", bb);
				config_add_value(&curr_conf, strdup(token), strdup(bb));
				token = bb;
			}
			token = strtok(NULL, delims);
		}
	}
	fclose(fp);
	/*printf("wwww- %d\n", strlen(curr_conf.vhost));
	printf("wwww2- %d\n", strlen("a.ge"));
	printf("cccc - %d\n", curr_conf.vhost[strlen(curr_conf.vhost)-1]);
	printf("pppppp - %d\n", strcmp(curr_conf.vhost, "a.ge"));
	printf("qqqqqq - %s\n", curr_conf.ip);*/
	HashSetEnter(configs, &curr_conf);
//	printf("answer = %s\n", ((struct config *)HashSetLookup(configs, "a.ge"))->ip);
	//printf("yyyyyy - %s\n", ((struct config *)HashSetLookup(configs, "a.ge"))->vhost);
}

vector * get_all_port_numbers(hashset *configs)
{
	vector *v = (vector *) malloc(sizeof(vector));
	assert(v);
	VectorNew(v, sizeof(char *), NULL, 4);

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
	VectorAppend((vector *)auxData, ((struct config *)elemAddr)->port);
}
