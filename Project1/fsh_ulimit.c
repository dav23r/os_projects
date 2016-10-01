#include "fsh_ulimit.h"

bool fsh_ulimit_helper(r_limit fn, char flag, int limit, char s_h_flag);

int get_rlim_cur(char flag, int limit, int resource){
    struct rlimit  rlim;
    if (limit == 0 ||flag=='H')
    if (getrlimit(resource, &rlim)<0){
        return -1;
    }else{
        return rlim.rlim_cur;
    }
    if (flag=='S')
        return  limit;
    return  limit;
}
int get_rlim_max(char flag, int limit, int resource){
    struct rlimit  rlim;
    if (limit == 0 ||flag=='S')
    if (getrlimit(resource, &rlim)<0){
        return -1;
    }else{
        return rlim.rlim_max;
    }
    if (flag=='H')
        return  limit;
    return  limit;
}

bool set_limit(char s_h_flag, int limit, int resource){
    printf("dfvdf");
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
//number by which it should be divided
//to show what bash would've shown
int resource_correspondence(int resource){
    if (resource==RLIMIT_CPU||resource==RLIMIT_NPROC||resource==RLIMIT_SIGPENDING||resource==RLIMIT_MSGQUEUE)
        return 1;
    if (resource==RLIMIT_NOFILE)
        return 512;
    return 1024;
}

bool get_limit(char s_h_flag, int limit, int resource){
    struct rlimit rl;
    rl.rlim_cur = get_rlim_cur(s_h_flag, limit,resource);
    if (rl.rlim_cur<0){
        printf("error while getting limit\n");
        return false;
    }else{
        if (rl.rlim_cur==RLIM_INFINITY)
            printf("unlimited\n");
        else
            printf("%ld\n",(long)rl.rlim_cur/resource_correspondence(resource));
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
        if (!flag_args || flag_args->num_args == 0) {
            limit = 0;
        } else {
            char *arg = flag_args->arguments[1];
            if (!is_valid_integer(arg)) {
                printf("invalid limit value (must be integer)\n");
                return false;
            }
            limit = atoi(arg);
        }
        fsh_ulimit_helper((limit == 0 ? get_limit : set_limit), flag, limit, soft_or_hard_limit);
    }
}

bool fsh_ulimit_helper(r_limit fn, char flag, int limit, char s_h_flag){
    struct rlimit rl;
    switch (flag){
        case 'a':
            fsh_ulimit_helper(fn,'c',limit, s_h_flag);
            fsh_ulimit_helper(fn,'d',limit, s_h_flag);
            fsh_ulimit_helper(fn,'e',limit, s_h_flag);
            fsh_ulimit_helper(fn,'f',limit, s_h_flag);
            fsh_ulimit_helper(fn,'i',limit, s_h_flag);
            fsh_ulimit_helper(fn,'l',limit, s_h_flag);
            fsh_ulimit_helper(fn,'m',limit, s_h_flag);
            fsh_ulimit_helper(fn,'p',limit, s_h_flag);
            fsh_ulimit_helper(fn,'q',limit, s_h_flag);
            fsh_ulimit_helper(fn,'r',limit, s_h_flag);
            fsh_ulimit_helper(fn,'s',limit, s_h_flag);
            fsh_ulimit_helper(fn,'t',limit, s_h_flag);
            fsh_ulimit_helper(fn,'u',limit, s_h_flag);
            fsh_ulimit_helper(fn,'v',limit, s_h_flag);
            fsh_ulimit_helper(fn,'x',limit, s_h_flag);
            break;
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
        case 'n': case 'p':
            fn(s_h_flag,limit,RLIMIT_NOFILE);
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
    }
}