#include "cgi_runner.h"
#include "sys/wait.h"
#include "unistd.h"
#include <assert.h>


void set_up_environment(struct header_info *http_header);

#define child ((pid_t) 0)
#define error ((pid_t) -1)
#define stdin_fd 0
#define stdout_fd 1
#define UNREACHEBLE false
bool run_cgi_script(struct header_info *http_header, 
                    int socket_fd, 
                    char *program_to_run){

    /* Assert existance of executable with given name. */
    if (access(program_to_run, X_OK) == -1) {
        perror ("Can't access executable!");
        return false;
    }

    /* Fork to execute script in child process. */
    int after_fork_fd = fork();
    if (after_fork_fd == error) {
        perror ("Can't fork!");
        return false;
    } else if (after_fork_fd == child) {
        /* Free current resourses associated with stdio/stdout. */
        close(stdout_fd);
        close(stdin_fd);
        /* Point both of them to the resourse socket_fd points to. */
        dup2(socket_fd, stdout_fd);
        dup2(socket_fd, stdin_fd);
        set_up_environment(http_header);
        execl(program_to_run, "cgi-script",  NULL);
        /* Later code will be executed only in case of error ocuring. */
        perror ("Error in child process!");
        return false;
    } else {
    /* Parent process waits for child to terminate. */
        // TODO: should we close the socket???
        assert( close(socket_fd) == 0);   // Let it go..
        return (wait(NULL) != -1);        // There is only one child 
    }

    return UNREACHEBLE;
}

/* Assigns values to variables specified by CGI protocol.
   This should happend in child process(after fork), which
   will be followed by jumping to script code block. */
#define do_overwrite 1
void set_up_environment(struct header_info *http_header){
     
    setenv("GATEWAY-INTERFACE", "CGI/1.1", do_overwrite);
    setenv("SERVER-PROTOCOL", "INCLUDED", do_overwrite);

    /*
    setenv("CONTENT-LENGTH", http_header->content_length, do_overwrite);
    setenv("CONTENT-TYPE", http_header->content_type, do_overwrite);
    setenv("REQUEST-METHOD", http_header->method, do_overwrite);
    */

}
