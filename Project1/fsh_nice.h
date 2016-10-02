//
// Created by irakli on 9/29/16.
//

#ifndef PROJECT1_FSH_NICE_H
#define PROJECT1_FSH_NICE_H

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "arguments.h"
#include <signal.h>
#include <sys/wait.h>

bool fsh_nice_helper(char flag, int increment, char * program_name, char * const argv[]);

bool fsh_nice(pos_arguments *args);

#endif //PROJECT1_FSH_NICE_H
