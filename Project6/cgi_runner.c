#include "cgi_runner.h"
#include "stdio.h"
#include "unistd.h"

void set_up_envioroment(header_infor *http_header);

#define child ((pid_t) 0)
#define error ((pid_t) -1)
#define stdin_fd 0
#define stdout_fd 1
#define UNREACHEBLE false
bool run_cgi_script(header_info *http_header, 
                    int socket_fd, 
                    char *program_to_run){

    /* Assert existance of executable with given name. */
    if (!access(program_to_run, X_OK)) {
        perror ("Can't access executable!");
        return false;
    }

    /* Fork to execute script in child process. */
    int after_fork_fd = fork();
    if (after_fork_fd = error) {
        perror ("Can't fork!");
        return false;
    } else if (after_fork_fd == child) {
        /* Free current resourses associated with stdio/stdout. */
        close(stdout_fd);
        close(stdin_fd);
        /* Point both of them to the resourse socket_fd points to. */
        dup2(socket_fd, stdout_fd);
        dup2(socket_fd, stdin_fd);
        set_up_enviorement(http_header);
        execv(program_to_run, NULL);
        /* Later code will be executed only in case of error ocuring. */
        perror ("Error in child process!");
        return false;
    } else {
    /* Parent process waits for child to terminate. */
        return (wait(NULL) == 0); // There is only one child 
    }
    return UNREACHEBLE;
}

/* Assigns values to variables specified by CGI protocol.
   This should happend in child process(after fork), which
   will be followed by jumping to script code block. */
void set_up_envioroment(header_info *http_header){}
