#ifndef WEBSERVER_H
#define WEBSERVER_H


int epoll_fd;
//possible struct for saving information in epoll
typedef struct {
    int fd;
    int otherThing;
} Data;

#define WORKERS_NUM 1024


#endif
