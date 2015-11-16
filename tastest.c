/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.1 TEST TAS
	gcc tastest.c tas64.S -lrt
	Prof. Hakner
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include "sem.h"

#define NUM_CHILDREN 8
#define NUM_ITER 1000000

int tas(volatile char *spinlock);

void nothing(int nope) {}

int my_procnum = 0;

int main() {
	signal(SIGUSR1, nothing);
	int mem = shm_open("/lol", O_RDWR | O_CREAT | O_TRUNC, 0766); //or MAP_ANONYMOUS
	size_t size = sizeof(unsigned long)*2;
	if(mem<0 || ftruncate(mem, size)<0) {
		perror("Cannot create shared memory");
		exit(-1);
	}
	unsigned long *test = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);
	if(test == MAP_FAILED) {
		perror("mmap failed");
		exit(-1);
	}
	printf("Initializing test value to zero\n");
	*test = 0L;

	int pid, child = 0;
	printf("Spawning %d children...\n", NUM_CHILDREN);
	while(child < NUM_CHILDREN) {
		printf("Birthing child %d...\n", child);
		pid = fork();
		if(pid == 0) {
			for(; pid<NUM_ITER; pid++) {
				(*test)++;
			}
			printf("Child %d incremented test value %d times.\n", child, NUM_ITER);
			exit(0);
		}
		child++;
	}
	while(wait(NULL) != -1); //wait for kids to die
	printf("Test value is now %lu, should be %d\n", *test, NUM_CHILDREN*NUM_ITER);

	printf("\nRepeating using TAS...\n\n");
	volatile char* spinlock = (char*) test+sizeof(unsigned long);
	*spinlock = 0;
	
	printf("Initializing test value to zero\n");
	*test = 0L; //reset
	child = 0; //reset
	printf("Spawning %d children...\n", NUM_CHILDREN);
	while(child < NUM_CHILDREN) {
		printf("Birthing child %d...\n", child);
		pid = fork();
		if(pid == 0) {
			for(; pid<NUM_ITER; pid++) {
				while(tas(spinlock)); //spin
				(*test)++;
				*spinlock = 0; //clear lock
			}
			printf("Child %d incremented test value %d times.\n", child, NUM_ITER);
			exit(0);
		}
		child++;
	}
	while(wait(NULL) != -1); //wait for kids to die
	printf("Test value is now %lu, should be %d\n", *test, NUM_CHILDREN*NUM_ITER);

	printf("\nRepeating using semaphore...\n\n");
	int smem = shm_open("/lol", O_RDWR | O_CREAT | O_TRUNC, 0766); //or MAP_ANONYMOUS
	size = sizeof(struct sem)+2;
	if(smem<0 || ftruncate(smem, size)<0) {
		perror("Cannot create shared memory");
		exit(-1);
	}
	struct sem* s = mmap(NULL, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	//struct sem* s = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, smem, 0);
	sem_init(s, 1);
	
	printf("Initializing test value to zero\n");
	*test = 0L; //reset
	child = 0; //reset
	printf("Spawning %d children...\n", NUM_CHILDREN);
	while(child < NUM_CHILDREN) {
		printf("Birthing child %d...\n", child);
		pid = fork();
		if(pid == 0) {
			for(; pid<NUM_ITER; pid++) {
				sem_wait(s);
				(*test)++;
				sem_inc(s);
			}
			printf("Child %d incremented test value %d times.\n", child, NUM_ITER);
			exit(0);
		}
		child++;
		my_procnum++;
	}
	while(wait(NULL) != -1); //wait for kids to die
	printf("Test value is now %lu, should be %d\n", *test, NUM_CHILDREN*NUM_ITER);

	return 0;
}