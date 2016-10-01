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

const long long STATUS_CODE = -123456789101112LL;
/*displays some info for free shell*/
bool fsh_info(pos_arguments *args);

/*changed directory*/
bool fsh_cd(pos_arguments *args);

bool fsh_cd_helper(char * dir);

/*displays working directory*/
bool fsh_pwd(pos_arguments *args);

bool fsh_echo_name(pos_arguments *args);

bool fsh_echo_name_helper(char * variable_name);

bool fsh_echo_string(pos_arguments *args);

bool fsh_echo_string_helper(char * str);

bool fsh_echo_export(pos_arguments *args);
/*returns last status code or 0 if none
 *were evoked. t should be STATUS_CODE
 * if the value must be updated.*/
long long fsh_echo_last_status(long long  t);

#endif
