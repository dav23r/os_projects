#ifndef SCAN_DOCUMENTS_DIRECTORY_H
#define SCAN_DOCUMENTS_DIRECTORY_H

#include <string.h>
#include "bool.h"
#include "config_service.h"

char * scan_and_print_directory(char *directory_path, char *doc_root, char *host, bool save_html_file);
char *replace(char *str);

static char * remove_prefix(char *str, char *prefix);

#endif
