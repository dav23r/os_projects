#ifndef SCAN_DOCUMENTS_DIRECTORY_H
#define SCAN_DOCUMENTS_DIRECTORY_H

#include <string.h>
#include "bool.h"
#include "config_service.h"

char * scan_and_print_directory(char *directory_path, char *doc_root, bool save_html_file);
char *replace(char *str);

static char * remove_prefix(char *str, char *prefix);
void strings_vector_free_fn(void *elem);

#endif
