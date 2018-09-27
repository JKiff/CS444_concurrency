#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include "mersenne.c"
#include <unistd.h>

// Constant for how large the buffer can be
#define BUFFER_SIZE 32

// Buffer node struct, holds value and variable for "consumption time"
struct bufferNode {
	int value;
	int hangTime;
};

// Create the buffer
struct bufferNode buffer[BUFFER_SIZE];
// Create the mutex for control of the buffer
pthread_mutex_t lock;
// Create a variable that keeps track of the size of the buffer
int buffer_index = -1;

// This function produces a random number with the mersenne method genrand_int32
// TODO: Add hardware randomization support from what the assignment says
int produceRandom(int lo, int hi) {
	return (abs(genrand_int32()) % (hi + 1 - lo)) + lo;
}

// This function takes the buffer and increments each item forward in the queue after an item is removed from the front of the queue
void reconfigure_queue() {
	for (int i = 0; i < buffer_index-1; i++) {
		buffer[i] = buffer[i+1];
	}
}

// This is the producer function, each producer is constantly running this.
void *producer(void *ptr) {
	pid_t prod_pid = pthread_self();
	// printf("Process ID for producer is: %d\n", prod_pid);

	while (1) {
		// Have the producer sleep a set amount of time 3-7 seconds
		int sleep_time = produceRandom(3, 7);
		// printf("Producer sleeping %d seconds\n", sleep_time);
		sleep(sleep_time);

		int complete = 0;
		// Create a new Buffer Node
		struct bufferNode new_buff_val;
		// Populate the Buffer Node
		new_buff_val.value = produceRandom(5, 100);
		new_buff_val.hangTime = produceRandom(2, 9);
		while (!complete) {
			// Attempt to get the lock
			pthread_mutex_lock(&lock);

			// If the buffer is full, wait for a consumer to consume and then retry
			if (buffer_index >= BUFFER_SIZE) {
				pthread_mutex_unlock(&lock);
				continue;
			}
			// Increment the buffer item size
			// printf("Producer created value %d, hang time %d seconds\n", new_buff_val.value, new_buff_val.hangTime);
			buffer_index++;
			// Put the item in the buffer
			buffer[buffer_index] = new_buff_val;
			complete = 1;
			printf("Producer produced value %d\n", new_buff_val.value);
			printf("Size of buffer: %d\n", buffer_index);
			// Remove the lock
			pthread_mutex_unlock(&lock);
		}
		// Sleep to let other threads execute
		sleep(1);
	}
}

// This is the consumer function, each consumer is constantly running this.
void *consumer(void *ptr) {
	pid_t cons_pid = pthread_self();
	// printf("Process ID for consumer is: %d\n", cons_pid);

	while (1) {
		int complete = 0;
		while (!complete) {
			// Attempt to get the lock
			pthread_mutex_lock(&lock);

			// If there are no items on the buffer, wait for a producer to add something
			if (buffer_index == -1) {
				pthread_mutex_unlock(&lock);
				continue;
			}
			// Get the values from the buffer item
			int sleep_time = buffer[0].hangTime;
			int buffer_val = buffer[0].value;
			// printf("Consumer got value %d, sleeping %d seconds\n", buffer_val, sleep_time);
			// "Consume" the value
			sleep(sleep_time);
			// Decrement and reconfigure the queue (remove an item from the front)
			buffer_index--;
			reconfigure_queue();
			complete = 1;
			printf("Consumer consumed value %d\n", buffer_val);
			printf("Size of buffer: %d\n", buffer_index);
			// Remove the lock
			pthread_mutex_unlock(&lock);
		}
		// Sleep to let other threads execute
		sleep(1);
	}
}

int main() {
	// Initialize the randomizer
	init_genrand(1);

	// Values to determine total producers and consumers
	int num_producers = 32;
	int num_consumers = 1;
	int total_threads = num_producers + num_consumers;
	// Array to hold each of the threads
	pthread_t threads[total_threads];

	// Create all of the producers
	for (int i = 0; i < num_producers; i++) {
		if (pthread_create(&threads[i], NULL, producer, NULL)) {
			fprintf(stderr, "Error creating producer thread.\n");
			return 1;
		}
	}

	// Create all of the consumers
	for (int i = num_producers; i < total_threads; i++) {
		if (pthread_create(&threads[i], NULL, consumer, NULL)) {
			fprintf(stderr, "Error creating producer thread.\n");
			return 1;
		}
	}

	// Wait for other threads to complete
	for (int i = 0; i < total_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
			fprintf(stderr, "Error joining thread.\n");
			return 2;
		}
	}
	return 0;
}
