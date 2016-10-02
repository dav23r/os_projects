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
#include "arguments.h"
#include "context.h"

/*
 * calls the kill syscall with passed
 * signal and pid. Raises error if
 * not possible to perform.
 * */
bool fsh_kill_helper(pid_t pid, int signal);
/*
 * Processes the passed args
 * and calls fsh_kill_helper
 * if no syntax error was made.
 *
 * */
bool fsh_kill(pos_arguments *args);



#endif //PROJECT1_FSH_KILL_H
