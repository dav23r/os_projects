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

bool fsh_cd_helper(char * dir);

/*displays working directory*/
bool fsh_pwd(pos_arguments *args);
/**
 * prints according to given variable
 * name. or prints error if an
 * incorrect call was made.
 * */
bool fsh_echo_name(pos_arguments *args);
//a helper functions to call putenv
bool fsh_echo_name_helper(char * variable_name);
/*prints given string, if null raises error*/
bool fsh_echo_string(pos_arguments *args);
//prints given string, basically
// a printf with extra steps
bool fsh_echo_string_helper(char * str);
/*
 * exports given variable
 * */
bool fsh_echo_export(pos_arguments *args);

bool fsh_echo_export_helper(char * name, char * value);

/*returns last status code or 0 if none
 *were evoked. t should be STATUS_CODE
 * if the value must be updated.*/
long long fsh_echo_last_status(long long  t);

bool fsh_echo(pos_arguments *args);

#endif
