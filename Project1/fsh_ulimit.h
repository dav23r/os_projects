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

/*sets or gets limits, depending on flag*/
bool fsh_ulimit(args_and_flags *rest);
//void fsh_ulimit(r_limit fn, char flag, int limit, char s_h_flag);
typedef bool (*r_limit)(char, int, int);

#endif