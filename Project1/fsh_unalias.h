#ifndef PROJECT1_FSH_UNALIAS_H
#define PROJECT1_FSH_UNALIAS_H
#include <unistd.h>
#include "util.h"
#include <stdio.h>
#include "arguments.h"
#include "context.h"

/* Implements shell built-in 'unalias' */
bool fsh_unalias(pos_arguments *args, context *context);

#endif

