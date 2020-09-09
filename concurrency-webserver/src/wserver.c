#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include <stdlib.h>
#include "common_threads.h"

#define EMPTY (0) // buffer slot has nothing in it

char default_root[] = ".";
char FIFO[] = "FIFO";
char SFF[] = "SFF";

int *buffer = NULL;
int buffer_size = 1;
int use_ptr = 0;  // tracks where next consume should come from
int fill_ptr = 0; // tracks where next produce should go to
int num_full = 0; // counts how many entries are full

int num_thread_pool_size = 1;

char *schedalg = NULL;

// used in producer/consumer signaling protocol
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *run(void *p);
void produce(int conn_fd);
void *consume(void *arg);

//
// ./wserver [-d <basedir>] [-p <portnum>]
//
int main(int argc, char *argv[])
{
	int c;
	char *root_dir = default_root;
	int port = 10000;

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
			buffer_size = atoi(optarg);
			break;
		case 's':
			schedalg = optarg;
			break;
		default:
			fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]\n");
			exit(1);
		}

	printf("\n!!!parameters:start!!!\n");
	printf("root_dir:%s, port:%d, thread_pool:%d, buffer_size:%d, schedalg:%s\n", root_dir, port, num_thread_pool_size, buffer_size, schedalg);
	printf("!!!parameters:end!!!\n");

	// run out of this directory
	chdir_or_die(root_dir);

	// init buffer
	buffer = malloc(buffer_size * sizeof(int));

	// init thread pool
	for (int i = 0; i < num_thread_pool_size; i++)
	{
		pthread_t p;
		Pthread_create(&p, NULL, consume, NULL);
	}

	// now, get to work
	int listen_fd = open_listen_fd_or_die(port);
	while (1)
	{
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *)&client_addr, (socklen_t *)&client_len);
		produce(conn_fd);
	}
	return 0;
}

void produce(int conn_fd)
{
	Mutex_lock(&m);
	while (num_full == buffer_size)
	{
		Cond_wait(&empty, &m);
	}
	// printf("\n####produce enter %d fill_ptr:%d\n", conn_fd, fill_ptr);
	buffer[fill_ptr] = conn_fd;
	fill_ptr = (fill_ptr + 1) % buffer_size;
	num_full++;
	Cond_signal(&fill);
	Mutex_unlock(&m);
	// printf("\n####produce leave %d fill_ptr:%d\n", conn_fd, fill_ptr);
}

void *consume(void *arg)
{
	while (1)
	{
		Mutex_lock(&m);
		while (num_full == 0)
		{
			Cond_wait(&fill, &m);
		}
		int conn_fd = buffer[use_ptr];
		// printf("\n####consume enter %d use_ptr:%d\n", conn_fd, use_ptr);
		buffer[use_ptr] = EMPTY;
		use_ptr = (use_ptr + 1) % buffer_size;
		num_full--;
		Cond_signal(&empty);
		Mutex_unlock(&m);

		run((void *)(long long)conn_fd);

		// printf("\n####consume leave %d use_ptr:%d\n", conn_fd, use_ptr);
	}

	return NULL;
}

void *run(void *arg)
{
	int conn_fd = (int)arg;
	printf("\n####thread %li %d\n", (unsigned long int)pthread_self(), conn_fd);
	request_handle(conn_fd);
	close_or_die(conn_fd);

	return NULL;
}