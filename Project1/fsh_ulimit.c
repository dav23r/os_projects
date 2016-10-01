#include "fsh_ulimit.h"

bool fsh_ulimit_helper(r_limit fn, char flag, int limit, char s_h_flag);
/*
 * A helper function that returns rlimit.rlim_cur
 * on demand, depending on flag passed.
 * -1 returned on error.
 * Passed limit is returned
 * if it must be set as rlim_cur
 * */

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
/*
 * A helper function that returns rlimit.rlim_max
 * on demand, depending on flag passed.
 * -1 returned on error.
 * Passed limit is returned
 * if it must be set as rlim_max
 * */

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
/*
 * description as in bash shell
 * for ulimit flags.
 * */
char * get_description(char flag){
    switch (flag){
        case 'c':
            return strdup("core file size         ");
        case 'd':
            return strdup("data seg size          ");
        case 'f':
            return strdup("fiel size              ");
        case 'e':
            return strdup("scheduling priority    ");
        case 'm':
            return strdup("max memory size        ");
        case 'n':
            return strdup("open files             ");
        case 'q':
            return strdup("POSIX message queues   ");
        case 'r':
            return strdup("reali-time priority    ");
        case 's':
            return strdup("stack size             ");
        case 't':
            return strdup("cpu time               ");
        case 'u':
            return strdup("max user process       ");
        case 'v':
            return strdup("virtual memory         ");
        case 'x':
            return strdup("file locks             ");
        case 'i':
            return strdup("pending signals        ");
        case 'l':
            return strdup("max locked memory      ");
        case 'p':
            return strdup("pipe size              ");
        default:
            return strdup("");
    }
}

/*
 * sets limit in accordance with
 * passed flag and resource.
 * returns false on error and true
 * on success. the last two arguments
 * are to be ignored, but are used to
 * make the code way less.
 * * */
bool set_limit(char s_h_flag, int limit, int resource, char flag, bool print_info){
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
int resource_correspondence(int resource, char flag){
    if (resource==RLIMIT_CPU||resource==RLIMIT_NPROC||resource==RLIMIT_SIGPENDING||resource==RLIMIT_MSGQUEUE)
        return 1;
    if (resource==RLIMIT_NOFILE&&flag=='p')
        return 128;
    if (resource==RLIMIT_NOFILE)
        return 1;
    return 1024;
}

/*
 * prints limit according to flags.
 * gets description to print more information
 * so that suer actually understands the info
 * he's receiving.
 * */
bool get_limit(char s_h_flag, int limit, int resource, char flag, bool print_info){
    struct rlimit rl;
    rl.rlim_cur = get_rlim_cur(s_h_flag, limit,resource);
    if (rl.rlim_cur<0){
        printf("error while getting limit\n");
        return false;
    }else{
        char * description = (print_info? get_description(flag):strdup(""));
        if (rl.rlim_cur==RLIM_INFINITY)
            printf("%sunlimited\n",description);
        else
            printf("%s%ld\n",description,(long)rl.rlim_cur/resource_correspondence(resource,flag));
        free(description);
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
/**
 * a helper function which is called from
 * fsh_ulimit wrapper.
 * */
bool fsh_ulimit_helper(r_limit fn, char flag, int limit, char s_h_flag){
    struct rlimit rl;
    switch (flag){
        case 'a':
            fn(s_h_flag,limit,RLIMIT_CORE,'c',true);
            fn(s_h_flag,limit,RLIMIT_DATA,'d',true);
            fn(s_h_flag,limit,RLIMIT_NICE,'e',true);
            fn(s_h_flag,limit,RLIMIT_FSIZE,'f',true);
            fn(s_h_flag,limit,RLIMIT_SIGPENDING,'i',true);
            fn(s_h_flag,limit,RLIMIT_MEMLOCK,'l',true);
            fn(s_h_flag,limit,RLIMIT_RSS,'m',true);
            fn(s_h_flag,limit,RLIMIT_NOFILE,'n',true);
            fn(s_h_flag,limit,RLIMIT_NOFILE,'p',true);
            fn(s_h_flag,limit,RLIMIT_MSGQUEUE,'q',true);
            fn(s_h_flag,limit,RLIMIT_RTPRIO,'r',true);
            fn(s_h_flag,limit,RLIMIT_STACK,'s',true);
            fn(s_h_flag,limit,RLIMIT_CPU,'t',true);
            fn(s_h_flag,limit,RLIMIT_NPROC,'u',true);
            fn(s_h_flag,limit,RLIMIT_AS,'v',true);
            fn(s_h_flag,limit,RLIMIT_LOCKS,'x',true);
            break;
        case 'c':
            fn(s_h_flag,limit,RLIMIT_CORE,flag,false);
            break;
        case 'd':
            fn(s_h_flag,limit,RLIMIT_DATA,flag,false);
            break;
        case 'f':
            fn(s_h_flag,limit,RLIMIT_FSIZE,flag,false);
            break;
        case 'e':
            fn(s_h_flag,limit,RLIMIT_NICE,flag,false);
            break;
        case 'm':
            fn(s_h_flag,limit,RLIMIT_RSS,flag,false);
            break;
        case 'n': case 'p':
            fn(s_h_flag,limit,RLIMIT_NOFILE,flag,false);
            break;
        case 'q':
            fn(s_h_flag,limit,RLIMIT_MSGQUEUE,flag,false);
            break;
        case 'r':
            fn(s_h_flag,limit,RLIMIT_RTPRIO,flag,false);
            break;
        case 's':
            fn(s_h_flag,limit,RLIMIT_STACK,flag,false);
            break;
        case 't':
            fn(s_h_flag,limit,RLIMIT_CPU,flag,false);
            break;
        case 'u':
            fn(s_h_flag,limit,RLIMIT_NPROC,flag,false);
            break;
        case 'v':
            fn(s_h_flag,limit,RLIMIT_AS,flag,false);
            break;
        case 'x':
            fn(s_h_flag,limit,RLIMIT_LOCKS,flag,false);
            break;
        case 'i':
            fn(s_h_flag,limit,RLIMIT_SIGPENDING,flag,false);
            break;
        case 'l':
            fn(s_h_flag,limit,RLIMIT_MEMLOCK,flag,false);
            break;
    }
}