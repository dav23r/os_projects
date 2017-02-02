#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>
#include "config_service.h"
#include "webserver.h"
#include "worker.h"
#include "scan_documents_directory.h"
#include "hash.h"


#define LISTENERS_BACKLOG_SIZE 256

void * net_events_handler(void *aux);
int main(int argc, const char* argv[]){
	if (argc < 2) perror("config file not provided");
	hashset configs;
	HashSetNew(&configs, sizeof(struct config), 4, hash, cmp, freeFn);
	printf ("%d\n", 1);
	save_config(argv[1], &configs);
	printf ("%d\n", 2);

	char *doc_path = get_config_value("a.ge", "documentroot", &configs);
	printf("%s\n", doc_path);
	scan_and_print_directory(doc_path, true);
	vector *ports = get_all_port_numbers(&configs);
	int ports_number = VectorLength(ports), i = 0;


			//creation of epoll
		if((epoll_fd = epoll_create1(0)) == -1) {
			perror("epoll_create");
			exit(EXIT_FAILURE);
		}

	// todo: run listener threads
	pthread_t *threads = (pthread_t *)calloc(ports_number + WORKERS_NUM, sizeof(*threads));	// ports_number listener threads, WORKERS_NUM worker threads
	assert(threads);
	for (; i < ports_number; ++i){
		printf("listener\n");
		pthread_create(threads + i, NULL, net_events_handler, VectorNth(ports, i));}

	// run WORKERS_NUM worker threads
	for (; i < ports_number + WORKERS_NUM; ++i){
		printf("worker\n");
		pthread_create(threads + i, NULL, work, &configs);
	}

	for (i=0; i < ports_number + WORKERS_NUM; i++){
		printf("joined\n");
		pthread_join(threads[i], NULL);
	}

	free(threads);

	return 0;
}

void * net_events_handler(void *aux)
{
	//https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
	int sockfd, newsockfd, portno, clilen;
	// char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;


	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Initialize socket structure */
	memset((void *) &serv_addr, 0, sizeof(serv_addr));
	portno = atoi((char *)aux);

	serv_addr.sin_family = AF_INET;

	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
printf("epoll = %d\n", epoll_fd);
	listen(sockfd, LISTENERS_BACKLOG_SIZE);
	clilen = sizeof(cli_addr);
	while (true) {
		printf("waiting for connect\n");
		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		printf("fd = %d\n", newsockfd);
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
		((Data *)(event.data.ptr))->fd = newsockfd;
		event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
		//fd-s damateba epoll-is siashi
		if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newsockfd, &event)) == -1) {
			perror("epoll_create");
			exit(EXIT_FAILURE);
		}
	}
	return NULL;
}
