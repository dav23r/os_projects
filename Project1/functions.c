#include "functions.h"


void fsh_info(){
    printf("This Free Shell was made by an awesome group of four cs undergraduates\n");
    printf("Enjoy while it lasts\n");
}


void fsh_cd(){
    char * dir = ;
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

void fsh_nice(){


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
}

void set_limit(char s_h_flag, int limit, int resource){
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

void fsh_ulimit(){
    char flag = ;
    int limit  = ;
    char s_h_flag = ;
    struct rlimit rl;
    switch (flag){
        case 'c':
            set_limit(s_h_flag,limit,RLIMIT_CORE);
            break;
        case 'd':
            set_limit(s_h_flag,limit,RLIMIT_DATA);
            break;
        case 'f':
            set_limit(s_h_flag,limit,RLIMIT_FSIZE);
            break;
        case 'e':
            set_limit(s_h_flag,limit,RLIMIT_NICE);
            break;
        case 'm':
            set_limit(s_h_flag,limit,RLIMIT_RSS);
            break;
        case 'n':
            set_limit(s_h_flag,limit,RLIMIT_NOFILE);
            break;
        case 'p':
            set_limit(s_h_flag,limit,RLIMIT_);
            break;
        case 'q':
            set_limit(s_h_flag,limit,RLIMIT_MSGQUEUE);
            break;
        case 'r':
            set_limit(s_h_flag,limit,RLIMIT_RTPRIO);
            break;
        case 's':
            set_limit(s_h_flag,limit,RLIMIT_STACK);
            break;
        case 't':
            set_limit(s_h_flag,limit,RLIMIT_CPU);
            break;
        case 'u':
            set_limit(s_h_flag,limit,RLIMIT_NPROC);
            break;
        case 'v':
            set_limit(s_h_flag,limit,RLIMIT_AS);
            break;
        case 'x':
            set_limit(s_h_flag,limit,RLIMIT_LOCKS);
            break;
        case 'i':
            set_limit(s_h_flag,limit,RLIMIT_SIGPENDING);
            break;
        case 'l':
            set_limit(s_h_flag,limit,RLIMIT_MEMLOCK);
            break;
    }









}