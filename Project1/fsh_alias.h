#ifndef PROJECT1_FSH_ALIAS_H
#define PROJECT1_FSH_ALIAS_H
#include <sys/types.h>
#include <unistd.h>
#include "util.h"
#include <stdio.h>
#include "arguments.h"
#include "context.h"

bool fsh_alias(pos_arguments *args, context *context);

/* Rest is private and should not be used directly */
void add_to_aliases(char *alias, char *prog_name, hashset *alias_set);

#endif
