#include "fifo.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define NUM_WRITERS 2
#define NUM_READERS 1
#define NUM_ITER 8192

int my_procnum = 0;
int pid;
int history[NUM_WRITERS];

int main() {
	struct fifo* ff = mmap(NULL, sizeof(struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fifo_init(ff);
	printf("Spawning %d writers...\n", NUM_WRITERS);
	while(my_procnum < NUM_WRITERS) {
		pid = fork();
		if(pid == 0) {
			printf("Created writer %d with pid %d\n", my_procnum, getpid());
			int i;
			for(i=1 ; i<NUM_ITER; i++) {
				printf("Writer %d writing %d", my_procnum, NUM_ITER*my_procnum+i);
				fifo_wr(ff, NUM_ITER*my_procnum + i);
			}
			printf("Writer %d wrote %d values.\n", my_procnum, NUM_ITER-1);
			// fifo_wr(ff, 0);
			exit(0);
		}
		my_procnum++;
	}



	printf("Spawning %d readers...\n", NUM_READERS);
	while(my_procnum < NUM_WRITERS+NUM_READERS) {
		pid = fork();
		if(pid == 0) {
			printf("Created reader %d with pid %d\n", my_procnum, getpid());
			unsigned long cur, last = 0;
			int wrongCount = 0;
			while(1) {
				cur = fifo_rd(ff);
				int wr_procnum = cur/NUM_ITER;
				unsigned long val = cur%NUM_ITER;
				printf("Read %lu (%lu from %d)\n", cur, val, wr_procnum);
				if(val-history[wr_procnum] != 1) {
					printf("Incorrect value for writer %d: %lu, expected %d!\tERROR\n", wr_procnum, val, history[wr_procnum]+1);
					wrongCount++;
					//exit(-1);
				} else {
					last = cur;
					history[wr_procnum] = val;
				}
				int i;
				int breakCount = 0;
				for(i=0; i<NUM_WRITERS; i++) {
					if(history[i] == NUM_ITER-1) {
						breakCount++;
					}
				}
				if(breakCount == NUM_WRITERS) {
					break;
				}
			}
			printf("Reader %d done. %d incorrect, last read %lu.\n",my_procnum, wrongCount, last);
			exit(0);
		}
		my_procnum++;
	}

	while(wait(NULL) != -1);
	printf("Done.\n");


	return 0;
}
