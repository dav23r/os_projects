#ifndef _functions_
#define _functions_
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "context.h"
#include "util.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "arguments.h"


/*displays some info for free shell*/
void fsh_info();
/*changed directory*/
void fsh_cd();
/*displays working directory*/
void fsh_pwd();
#endif
