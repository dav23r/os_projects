#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, const char* argv[]){
  //https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int  n;

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
  //TODO replace with real IP
  inet_aton("63.161.169.137", &serv_add.sin_addr.s_addr);
  serv_addr.sin_port = htons(portno);

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

  /* If connection is established then start communicating */
  memset(buffer,0,256);
  n = read( newsockfd,buffer,255 );

  if (n < 0) {
    perror("ERROR reading from socket");
    exit(1);
  }

  printf("Here is the message: %s\n",buffer);

  /* Write a response to the client */
  n = write(newsockfd,"I got your message",18);

  if (n < 0) {
    perror("ERROR writing to socket");
    exit(1);
  }

  return 0;



}

