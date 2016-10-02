
#ifndef PROJECT1_FSH_TYPE_H
#define PROJECT1_FSH_TYPE_H

#include "arguments.h"
#include "util.h"
#include "hashset.h"
#include "context.h"

bool fsh_type(bool has_a_flag, pos_arguments *args, context *cont);

/* Helper routine, not intended as part of interface */
bool look_up_hashset(hashset *set, char *cur_token, const char *set_name);

#endif
