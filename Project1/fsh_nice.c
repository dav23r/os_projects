//
// Created by irakli on 9/29/16.
//
#include "fsh_nice.h"

bool fsh_nice(char flag, int increment, char * program_name, char * const argv[]){
    //getpriority can return negative values

    errno = 0;
    if (flag!='n'){
        int prior = getpriority(PRIO_PROCESS,getpid());
        if (errno!=0){
            error_handler(errno,"getting niceness");
            return;
        }else {
            printf("%d\n", prior);
            return;
        }
    }
    //TLPI
    pid_t childPid;
    switch (childPid = fork()) {
        case -1:
            printf("error while trying to fork");
            return;
        case 0:
            errno = 0;
            if (nice(increment)<0) {
                error_handler(errno, "nice");
                return;
            }
            if (execvp(program_name,argv)<0){
                error_handler(errno, "executing given program");
                return;
            }
            exit(-1);
            break;
        default:
            if (wait(NULL)==-1){
                error_handler(errno, "waiting for child to terminate");
                return;
            }

            break;
    }

}
