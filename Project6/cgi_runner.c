#include "cgi_runner.h"
#include "sys/wait.h"
#include "unistd.h"
#include <assert.h>
#include "url_tools.h"
#include <sys/sendfile.h>

void set_up_environment(struct header_info *http_header,
                        struct config *conf);

#define child ((pid_t) 0)
#define error ((pid_t) -1)
#define stdin_fd 0
#define stdout_fd 1
#define UNREACHEBLE false

bool run_cgi_script(struct header_info *http_header,
                    int socket_fd, struct config *conf){

    char *program_to_run = http_header->requested_objname;

    /* Assert existance of executable with given name. */
    if (access(program_to_run, X_OK) == -1) {
        printf ("%s\n", program_to_run);
        perror ("Can't access executable!");
        return false;
    }

    /* Fork to execute script in child process. */
    int after_fork_fd = fork();
    if (after_fork_fd == error) {
        perror ("Can't fork!");
        return false;
    } else if (after_fork_fd == child) {
        /* Point both of them to the resourse socket_fd points to. */
        assert (dup2(socket_fd, stdout_fd) != -1);
        assert (dup2(socket_fd, stdin_fd) != -1);

        set_up_environment(http_header, conf);
        execl(program_to_run, "cgi-script",  NULL);
        /* Later code will be executed only in case of error ocuring. */
        perror ("Error in child process!");
        return false;
    } else {
    /* Parent process waits for child to terminate. */
        int status = wait(NULL);    // There is only one child
        return (status != -1);
    }

    return UNREACHEBLE;
}

/* Assigns values to variables specified by CGI protocol.
   This should happend in child process(after fork), which
   will be followed by jumping to script code block. */
#define do_overwrite 1
void set_up_environment(struct header_info *http_header,
                        struct config *conf) {

    setenv("GATEWAY_INTERFACE", "CGI/1.1", do_overwrite);
    setenv("SERVER_PROTOCOL", "INCLUDED", do_overwrite);
    setenv("SERVER_NAME", conf->vhost, do_overwrite);
    setenv("SERVER_PORT", conf->port, do_overwrite);

    setenv("DOCUMENT_ROOT", conf->document_root, do_overwrite);
    setenv("REQUEST_METHOD",
          (http_header->method == GET) ? "GET" : "POST",  do_overwrite);
    setenv("PATH_INFO", http_header->path_info, do_overwrite);
    setenv("QUERY_STRING", http_header->query_string == NULL ? "" : http_header->query_string, do_overwrite);

    /* Path translated constitues of url encoded PATH_INFO appended to doc-root. */
    int len_path_info = strlen(http_header->path_info);
    int len_doc_root  = strlen(conf->document_root);
    char buff[len_doc_root + len_path_info + 1];
    strcpy(buff, conf->document_root);
    assert( uri_to_url(http_header->path_info,
                       buff + len_doc_root,
                       len_path_info) );

    setenv("PATH_TRANSLATED", buff, do_overwrite);

    setenv("CONTENT_TYPE", http_header->content_type == NULL ? "" : http_header->content_type, do_overwrite);
    setenv("CONTENT_LENGTH", http_header->content_length == NULL ? "" : http_header->content_length, do_overwrite);
}
