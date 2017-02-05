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
#include <signal.h>


#define LISTENERS_BACKLOG_SIZE 256

// args struct for listener threads
struct listener_thread_args {
	char **port;
	hashset *confs;
};

void * net_events_handler(void *aux);
int main(int argc, const char* argv[]){
	if (argc < 2) perror("config file not provided");
	signal(SIGPIPE, SIG_IGN);
	hashset configs;
	HashSetNew(&configs, sizeof(struct config), 4, hash, cmp, freeFn);
	save_config(argv[1], &configs);
	vector *ports = get_all_port_numbers(&configs);
	int ports_number = VectorLength(ports), i = 0;
	if((epoll_fd = epoll_create1(0)) == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}
	pthread_t *threads = (pthread_t *)calloc(ports_number + WORKERS_NUM, sizeof(*threads));	// ports_number listener threads, WORKERS_NUM worker threads
	assert(threads);
	struct listener_thread_args buffer[ports_number];
	for (; i < ports_number; ++i){
		struct listener_thread_args args;
		args.port = VectorNth(ports, i);
		args.confs = &configs;
		memcpy(&buffer[i], &args, sizeof(struct listener_thread_args));
		pthread_create(threads + i, NULL, net_events_handler, &buffer[i]);
	}
	// run WORKERS_NUM worker threads
	for (; i < ports_number + WORKERS_NUM; ++i){
		pthread_create(threads + i, NULL, work, &configs);
	}
	for (i=0; i < ports_number + WORKERS_NUM; i++){
		pthread_join(threads[i], NULL);
	}
	free(threads);
	VectorDispose(ports);
	return 0;
}

// listens for the given port and adds accepted fd-s in epoll
void * net_events_handler(void *aux)
{
	struct listener_thread_args *args = (struct listener_thread_args *)aux;
	hashset *configs = args->confs;
	char *vhost = get_vhost(configs, *(char **)(args->port));
	char *doc_path = get_config_value(vhost, "documentroot", configs);
	scan_and_print_directory(doc_path, doc_path, true);

	//https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
	int sockfd, newsockfd, portno, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Initialize socket structure */
	memset((void *) &serv_addr, 0, sizeof(serv_addr));
	portno = atoi(*(char **)(args->port));

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
	listen(sockfd, LISTENERS_BACKLOG_SIZE);
	clilen = sizeof(cli_addr);
	while (true) {
		printf("waiting for connect\n");
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
