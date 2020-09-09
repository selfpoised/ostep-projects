#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include <stdlib.h> 
#include "common_threads.h"

char default_root[] = ".";
char FIFO[] = "FIFO";
char SFF[] = "SFF";

void *run(void *p);

//
// ./wserver [-d <basedir>] [-p <portnum>]
//
int main(int argc, char *argv[])
{
	int c;
	char *root_dir = default_root;
	int port = 10000;
	int num_thread_pool_size = 1;
	int queue_size = 1;
	char *schedalg = NULL;

	while ((c = getopt(argc, argv, "d:p:t:b:s")) != -1)
		switch (c)
		{
		case 'd':
			root_dir = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 't':
			num_thread_pool_size = atoi(optarg);
			break;
		case 'b':
			queue_size = atoi(optarg);
			break;
		case 's':
			schedalg = optarg;
			break;
		default:
			fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
			exit(1);
		}

	// run out of this directory
	chdir_or_die(root_dir);

	// now, get to work
	int listen_fd = open_listen_fd_or_die(port);
	while (1)
	{
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *)&client_addr, (socklen_t *)&client_len);

		pthread_t p;
		Pthread_create(&p, NULL, run, (void *) (long long) conn_fd);
	}
	return 0;
}

void *run(void *arg) {
	printf("\n####thread %li\n", (unsigned long int)pthread_self());
    int conn_fd = (int) arg;
	request_handle(conn_fd);
	close_or_die(conn_fd);

	return NULL;
}