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
    printf("%s\n", args->arguments[1]);
    printf("%s\n", args->arguments[2]);
    printf("%d\n", args->num_args);

    if (!args || !args->arguments) {
        printf("syntax error in calling 'nice'\n");
        return false;
    }


    if (args->num_args < 2 && !strcmp(args->arguments[0], "nice"))
        return fsh_nice_helper('0', 10, NULL, NULL);


    if (args->num_args == 2 && !strcmp(args->arguments[0], "nice")) {
        char *argv[] = {args->arguments[1], NULL};
        return fsh_nice_helper('n', 10, args->arguments[1], argv);
    }

    if (!strcmp(args->arguments[0], "nice") && (args->num_args < 4 || strcmp(args->arguments[1], "-n") != 0 || !is_valid_integer(args->arguments[3]))) {
        printf("syntax error in passing arguments in 'nice'\n");
        return false;
    }

    char **argv = get_changed_copy_array(args->num_args - 3, args, &args->arguments[2]);
    /*printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);*/

    bool res = fsh_nice_helper('n', atoi(args->arguments[3]), args->arguments[0], argv);
    free(argv);
    return res;
}

bool fsh_nice_helper(char flag, int increment, char * program_name, char * const argv[]){
     printf("%c\n", flag);
      printf("%d\n", increment);
    printf("%s\n", program_name);
    printf("%s\n", argv[0]);
     printf("%s\n", argv[1]);
      printf("%d\n", argv[2] == NULL);
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

