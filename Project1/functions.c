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


void fsh_echo_name(char * variable_name){
    char * res;
    res = getenv(variable_name);
    if (res==NULL){
        printf("\n");
    }else{
        printf("%s\n",res);
    }
}

void fsh_echo_string(char * str){
    printf("%s\n",str);
}

void fsh_echo_export(){



}







