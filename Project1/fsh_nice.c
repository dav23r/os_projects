//
// Created by irakli on 9/29/16.
//
#include "fsh_nice.h"

char ** get_changed_copy_array(int len, pos_arguments *args, char **funcname, int diff) {

    char ** argv = malloc((len + 2) * sizeof(char *));
    argv[0] = *funcname;
    int i = 0;
    for (; i < len; i++) {
        argv[i+1] = args->arguments[i+diff];
    }
    argv[len+1] = NULL;

    return argv;
}

bool fsh_nice(pos_arguments *args) {
    printf("zero = %s\n", args->arguments[0]);
    if (!args || !args->arguments) {
        printf("syntax error in calling 'nice'\n");
        return false;
    }


    if (args->num_args < 2 && !strcmp(args->arguments[0], "nice"))
        return fsh_nice_helper('0', 10, NULL, NULL);

    char flag = 'n';
    char *program_name;
    int increment = 10;
    char **argv;

    if (!strcmp(args->arguments[0], "nice")) {
        printf("%s\n", "aa");
        if (!strcmp(args->arguments[1], "-n")) {
            printf("%s\n", "bb");
            if (args->num_args < 3 || !is_valid_integer(args->arguments[2]) || args->num_args < 4) {
                printf("%s\n", "ccc");
                printf("syntax error in passing arguments in 'nice'\n");
                return false;
            } else {
                printf("%s\n", "ddd");
                program_name = args->arguments[3];
                increment = atoi(args->arguments[2]);
                argv = get_changed_copy_array(args->num_args - 4, args, &program_name, 4);
            }
        } else {
            printf("%s\n", "vvv");
            program_name = args->arguments[1];
            argv = get_changed_copy_array(args->num_args - 2, args, &program_name, 2);
        }
        
    }

    if (args->num_args < 1 || !args->arguments) {
        printf("syntax error in passing arguments in 'nice'\n");
        return false;
    }

    program_name = args->arguments[0];
    argv = get_changed_copy_array(args->num_args - 1, args, &program_name, 1);

    printf("flag = %c\n", flag);
    printf("inc = %d\n", increment);
    printf("name = %s\n", program_name);
    printf("%d\n", argv == NULL);
    bool res = fsh_nice_helper(flag, increment, program_name, argv);
    free(argv);
    return res;
}

bool fsh_nice_helper(char flag, int increment, char * program_name, char * const argv[]){
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

