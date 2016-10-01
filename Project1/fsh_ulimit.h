#ifndef _fsh_ulimit_
#define _fsh_ulimit_
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "arguments.h"
#include <stdlib.h>
#include <string.h>

/*
 * an implementation of ulimit for free shell
 * to see what the flags mean type help ulimit
 * sets or gets limits, depending on flag
 * */
bool fsh_ulimit(args_and_flags *rest);

typedef bool (*r_limit)(char, int, int, char,bool);

#endif