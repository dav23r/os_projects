//
// Created by irakli on 9/27/16.
//

#ifndef PROJECT1_UTIL_C_H
#define PROJECT1_UTIL_C_H

//error handling from tutorialspoint.com
void error_handler(int errnum, char * errmsg){
    fprintf(stderr, "Value of errno: %d\n", errnum);
    perror("Error printed by perror");
    fprintf(stderr, "Error while executing %s: %s\n",errmsg, strerror( errnum ));
}











#endif //PROJECT1_UTIL_C_H
