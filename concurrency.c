#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include "mersenne.c"
#include <unistd.h>

#define BUFFER_SIZE 32

struct bufferNode {
	int value;
	int hangTime;
};

struct bufferNode buffer[BUFFER_SIZE];
pthread_mutex_t lock;
int buffer_index = -1;

int produceRandom(int lo, int hi) {
	return (abs(genrand_int32()) % (hi + 1 - lo)) + lo;
}

void reconfigure_queue() {
	for (int i = 0; i < buffer_index-1; i++) {
		buffer[i] = buffer[i+1];
	}
}

void *producer(void *ptr) {
	pid_t prod_pid = pthread_self();
	// printf("Process ID for producer is: %d\n", prod_pid);

	while (1) {
		int sleep_time = produceRandom(3, 7);
		// printf("Producer sleeping %d seconds\n", sleep_time);
		sleep(sleep_time);

		int complete = 0;
		struct bufferNode new_buff_val;
		new_buff_val.value = produceRandom(5, 100);
		new_buff_val.hangTime = produceRandom(2, 9);
		while (!complete) {
			pthread_mutex_lock(&lock);

			if (buffer_index >= BUFFER_SIZE) {
				pthread_mutex_unlock(&lock);
				continue;
			}
			// printf("Producer created value %d, hang time %d seconds\n", new_buff_val.value, new_buff_val.hangTime);
			buffer_index++;
			buffer[buffer_index] = new_buff_val;
			complete = 1;
			printf("Producer produced value %d\n", new_buff_val.value);
			printf("Size of buffer: %d\n", buffer_index);
			pthread_mutex_unlock(&lock);
		}
		sleep(1);
	}
}

void *consumer(void *ptr) {
	pid_t cons_pid = pthread_self();
	// printf("Process ID for consumer is: %d\n", cons_pid);

	while (1) {
		int complete = 0;
		while (!complete) {
			pthread_mutex_lock(&lock);

			if (buffer_index == -1) {
				pthread_mutex_unlock(&lock);
				continue;
			}
			int sleep_time = buffer[0].hangTime;
			int buffer_val = buffer[0].value;
			// printf("Consumer got value %d, sleeping %d seconds\n", buffer_val, sleep_time);
			sleep(sleep_time);
			buffer_index--;
			reconfigure_queue();
			complete = 1;
			printf("Consumer consumed value %d\n", buffer_val);
			printf("Size of buffer: %d\n", buffer_index);
			pthread_mutex_unlock(&lock);
		}
		sleep(1);
	}
}

int main() {
	init_genrand(1);
	// for (int i = 0; i < 100; i++) {
	// 	printf("%d\n", produceRandom(2, 9));
	// }

	int num_producers = 32;
	int num_consumers = 1;
	int total_threads = num_producers + num_consumers;
	pthread_t threads[total_threads];

	for (int i = 0; i < num_producers; i++) {
		if (pthread_create(&threads[i], NULL, producer, NULL)) {
			fprintf(stderr, "Error creating producer thread.\n");
			return 1;
		}
	}

	for (int i = num_producers; i < total_threads; i++) {
		if (pthread_create(&threads[i], NULL, consumer, NULL)) {
			fprintf(stderr, "Error creating producer thread.\n");
			return 1;
		}
	}

	for (int i = 0; i < total_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
			fprintf(stderr, "Error joining thread.\n");
			return 2;
		}
	}
	return 0;
}
