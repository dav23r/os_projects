#include "functions.h"

typedef void (*r_limit)(char, int, int);
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

void fsh_nice(char flag, int increment){
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
            if (nice(increment)<0)
                error_handler(errno,"nice");


            break;
        default:
            sleep(3);
            break;
    }

    if child
        execve(processed path);
        nice();
    return;



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

void set_limit(char s_h_flag, int limit, int resource){
    struct rlimit rl;
    rl.rlim_cur = get_rlim_cur(s_h_flag, limit,resource);
    rl.rlim_max = get_rlim_max(s_h_flag, limit,resource);
    if (rl.rlim_cur<0||rl.rlim_max<0){
        printf("error while getting limit\n");
        return;
    }
    if (setrlimit(resource, &rl) == -1){
        printf("error while setting limit\n");
        return;
    }
}
void get_limit(char s_h_flag, int limit, int resource){
    struct rlimit rl;
    rl.rlim_cur = get_rlim_cur(s_h_flag, limit,resource);
    if (rl.rlim_cur<0){
        printf("error while getting limit\n");
        return;
    }else{
        printf("%d\n",rl.rlim_cur);
    }
}


void fsh_ulimit(r_limit fn, char flag, int limit, char s_h_flag){
    char flag = ;
    int limit  = ;
    char s_h_flag = ;
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