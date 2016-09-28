#ifndef _functions_
#define _functions_
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "context.h"
#include "util.h"
#include <stdlib.h>

/*displays some info for free shell*/
void fsh_info();
/*changed directory*/
void fsh_cd();
/*displays working directory*/
void fsh_pwd();
/*returns rlim cur for given flag and given limit*/
int get_rlim_cur(char flag, int limit, int resource);
/*returns rlim max for given flag and given limit*/
int get_rlim_max(char flag, int limit, int resource);



#endif
