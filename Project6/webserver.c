#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include "config_service.h"

int epoll_fd;
//possible struct for saving information in epoll
typedef struct {
    int fd;
    int otherThing;
} Data;



int main(int argc, const char* argv[]){
  if (argc < 1) on_error("config file not provided");
  hashset map;
  save_config(argv[0], &map);

  //https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int  n;
  //creation of epoll
  if((epoll_fd = epoll_create1(0)) == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }


  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  /* Initialize socket structure */
  memset((void *) &serv_addr, 0, sizeof(serv_addr));
  //must be read from files
  portno = 5001;

  serv_addr.sin_family = AF_INET;

  serv_add.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(portno);
  int val = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))<0){
    perror("ERROR on setting socket options");
    exit(1);
  }
  /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR on binding");
    exit(1);
  }

  /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
  */

  //TODO change 128 to some constant
  listen(sockfd,128);
  clilen = sizeof(cli_addr);

  /* Accept actual connection from the client */
  newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

  if (newsockfd < 0) {
    perror("ERROR on accept");
    exit(1);
  }
  struct epoll_event event;
  // event.data.ptr how you attach some persistent data to a fd in epoll
  if((event.data.ptr = malloc(sizeof(Data))) == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  event.events = EPOLLIN | EPOLLLT | EPOLLONESHOT;
  //fd-s damateba epoll-is siashi
  if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newsockfd, &event)) == -1) {
    perror("epoll_create");
    exit(EXIT_FAILURE);
  }



  return 0;



}

