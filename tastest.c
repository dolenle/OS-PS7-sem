/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.1 TEST TAS
	gcc tastest.c -lrt
	Prof. Hakner
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define NUM_CHILDREN 4
#define NUM_ITER 2000000

int tas(volatile char *spinlock);

int main() {
	int mem = shm_open("/tas_share", O_RDWR | O_CREAT | O_TRUNC, 0766);
	size_t size = sizeof(unsigned long)*2;
	if(mem<0 || ftruncate(mem, size)<0) {
		perror("Cannot create shared memory");
		exit(-1);
	}
	unsigned long *test = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);
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

	return 0;
}