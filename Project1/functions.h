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
bool fsh_info(pos_arguments *args);

/*changed directory*/
bool fsh_cd(pos_arguments *args);

void fsh_cd_helper(char * dir);

/*displays working directory*/
bool fsh_pwd(pos_arguments *args);

bool fsh_echo_name(pos_arguments *args);

void fsh_echo_name_helper(char * variable_name);

bool fsh_echo_string(pos_arguments *args);

void fsh_echo_string_helper(char * str);

bool fsh_echo_export(pos_arguments *args);


#endif
