//
// Created by irakli on 9/29/16.
//

#include "fsh_kill.h"

void fsh_kill(pid_t pid, int signal){
    errno = 0;
    if (kill(pid, signal)<0)
        error_handler(errno,"killing process");
}