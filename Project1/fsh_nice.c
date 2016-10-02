//
// Created by irakli on 9/29/16.
//
#include "fsh_nice.h"

char ** get_changed_copy_array(int len, pos_arguments *args, char **funcname) {

    char ** argv = malloc((len + 2) * sizeof(char *));
    argv[0] = *funcname;
    int i = 0;
    for (; i < len; i++) {
        argv[i+1] = args->arguments[i+3];
    }
    argv[len+1] = NULL;

    return argv;
}

bool fsh_nice(pos_arguments *args) {

    printf("%s\n", args->arguments[0]);

    if (!args || !args->arguments || (args->num_args != 1 && ((args->num_args > 0 && args->num_args < 3)
            ||(args->num_args > 1 && strcmp(args->arguments[1], "-n") != 0)
            || (args->num_args > 2 && is_valid_integer(args->arguments[2]))))) {
        printf("syntax error in calling 'nice'\n");
        return false;
    }


    if (args->num_args < 1)
        return fsh_nice_helper('0', 10, NULL, NULL);


    if (args->num_args == 1) {
        char *argv[] = {args->arguments[0], NULL};
        return fsh_nice_helper('n', 10, args->arguments[0], argv);
    }

    char **argv = get_changed_copy_array(args->num_args - 3, args, &args->arguments[0]);

    bool res = fsh_nice_helper('n', atoi(args->arguments[1]), args->arguments[0], argv);
    free(argv);
    return res;
}

bool fsh_nice_helper(char flag, int increment, char * program_name, char * const argv[]){
    //getpriority can return negative values
    errno = 0;
    if (flag!='n'){
        int prior = getpriority(PRIO_PROCESS,getpid());
        if (errno!=0){
            error_handler(errno,"getting niceness");
            return false;
        }else {
            printf("%d\n", prior);
            return true;
        }
    }
    //TLPI
    pid_t childPid;
    switch (childPid = fork()) {
        case -1:
            printf("error while trying to fork");
            return false;
        case 0:
            errno = 0;
            if (nice(increment)<0)
                error_handler(errno, "nice");
            else if (execvp(program_name,argv)<0)
                error_handler(errno, "executing given program");
            exit(-1);
            break;
        default:
            if (wait(NULL)==-1){
                error_handler(errno, "waiting for child to terminate");
                return false;
            }
            break;
    }
    return true;
}

