#include "functions.h"


void fsh_info(){
    printf("This Free Shell was made by an awesome group of four cs undergraduates\n");
    printf("Enjoy while it lasts\n");
}


void fsh_cd(char * dir){
    if (chdir(dir)<0){
        error_handler(errno," changing directory ");
    }
}


void fsh_pwd(){
    //glibc standard
    char * cur_dir = getcwd(NULL, 0);
    if (cur_dir==NULL) {
        error_handler(errno, " getting working directory ");
    }else {
        printf("%s\n", cur_dir);
        free(cur_dir);
    }
}

void fsh_nice(char flag, int increment, char * program_name, char * const argv[]){
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
            break;
        default:
            if (wait(NULL)==-1){
                error_handler(errno, "waiting for child to terminate");
                return;
            }

            break;
    }

}

void fsh_getenv(){
    char * name = ;
    char * res;
    res = getenv(name);
    if (res==NULL){
        printf("\n");
    }else{
        printf("%s\n",res);
    }
}


int get_rlim_cur(char flag, int limit, int resource){
    if (flag=='S')
        return  limit;
    struct rlimit  rlim;
    if (flag=='H')
        if (getrlimit(resource, &rlim)<0){
            return -1;
        }else{
            return rlim.rlim_cur;
        }
    return  limit;
}
int get_rlim_max(char flag, int limit, int resource){
    if (flag=='H')
        return  limit;
    struct rlimit  rlim;
    if (flag=='S')
        if (getrlimit(resource, &rlim)<0){
            return -1;
        }else{
            return rlim.rlim_max;
        }
    return  limit;
}

bool set_limit(char s_h_flag, int limit, int resource){
    struct rlimit rl;
    rl.rlim_cur = get_rlim_cur(s_h_flag, limit,resource);
    rl.rlim_max = get_rlim_max(s_h_flag, limit,resource);
    if (rl.rlim_cur<0||rl.rlim_max<0){
        printf("error while getting limit\n");
        return false;
    }
    if (setrlimit(resource, &rl) == -1){
        printf("error while setting limit\n");
        return false;
    }

    return true;
}
bool get_limit(char s_h_flag, int limit, int resource){
    struct rlimit rl;
    rl.rlim_cur = get_rlim_cur(s_h_flag, limit,resource);
    if (rl.rlim_cur<0){
        printf("error while getting limit\n");
        return false;
    }else{
        printf("%d\n",rl.rlim_cur);
    }

    return true;
}

// checks if arg contains only digits
bool is_valid_integer(char *arg) {
    int i;
    for (i = 0; i < strlen(arg); i++) {
        if (arg[i] < '0' || arg[i] > '9' || i > 9)  // also if larger than integer can store
            return false;
    }
    return true;
}

/*
 * Checks if user asks for soft limit, hard limit or both;
 * only -H means hard limit.
 * only -S or nothinf meeans soft limit.
 * both flags means both limits and we return 'B' as special character for caller.
 */
char find_limit_type(args_and_flags *rest) {
    bool soft_found = false;
    bool hard_found = false;

    int i;
    for (i = 0; i < rest->num_flags; i++) {
        char flag = rest->flags[i].flag;
        if (flag == 'S')
            soft_found = true;
        else if (flag == 'H')
            hard_found = true;
    }

    if ((!soft_found && !hard_found) || soft_found && !hard_found) return 'S';
    if (soft_found && hard_found) return 'B'; // both
    return 'H';
}

bool fsh_ulimit(args_and_flags *rest) {
    int i;
    char soft_or_hard_limit = find_limit_type(rest);
    for (i = 0; i < rest->num_flags; i++) {
        char flag = rest->flags[i].flag;
        if (flag == 'S' || flag == 'H') continue;
        
        pos_arguments *flag_args = rest->flags[i].flag_arguments;
        int limit;

        if (pos_arguments->num_args == 0) {
            limit = 0;
        } else {
            char *arg = flag_args->arguments[1];
            if (!is_valid_integer(arg)) {
                printf("invalid limit value (must be integer)\n");
                return false;
            }
            limit = atoi(arg);
        }

        fsh_ulimit_helper((limit == 0 ? set_limit : get_limit), flag, limit, soft_or_hard_limit);
    }
}

bool fsh_ulimit_helper(r_limit fn, char flag, int limit, char s_h_flag){
    struct rlimit rl;
    switch (flag){
        case 'c':
            fn(s_h_flag,limit,RLIMIT_CORE);
            break;
        case 'd':
            fn(s_h_flag,limit,RLIMIT_DATA);
            break;
        case 'f':
            fn(s_h_flag,limit,RLIMIT_FSIZE);
            break;
        case 'e':
            fn(s_h_flag,limit,RLIMIT_NICE);
            break;
        case 'm':
            fn(s_h_flag,limit,RLIMIT_RSS);
            break;
        case 'n':
            fn(s_h_flag,limit,RLIMIT_NOFILE);
            break;
        case 'p':
            fn(s_h_flag,limit,RLIMIT_);
            break;
        case 'q':
            fn(s_h_flag,limit,RLIMIT_MSGQUEUE);
            break;
        case 'r':
            fn(s_h_flag,limit,RLIMIT_RTPRIO);
            break;
        case 's':
            fn(s_h_flag,limit,RLIMIT_STACK);
            break;
        case 't':
            fn(s_h_flag,limit,RLIMIT_CPU);
            break;
        case 'u':
            fn(s_h_flag,limit,RLIMIT_NPROC);
            break;
        case 'v':
            fn(s_h_flag,limit,RLIMIT_AS);
            break;
        case 'x':
            fn(s_h_flag,limit,RLIMIT_LOCKS);
            break;
        case 'i':
            fn(s_h_flag,limit,RLIMIT_SIGPENDING);
            break;
        case 'l':
            fn(s_h_flag,limit,RLIMIT_MEMLOCK);
            break;
        case 'T':
            fn(s_h_flag,limit,RLIMIT_NTHR);
    }









}