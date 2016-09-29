//
// Created by irakli on 9/29/16.
//

#ifndef PROJECT1_FSH_KILL_H
#define PROJECT1_FSH_KILL_H
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <stdio.h>

void fsh_kill(pid_t pid, int signal);


#endif //PROJECT1_FSH_KILL_H
