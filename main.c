/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.4 FIFO acid test
	Prof. Hakner
*/

#include "fifo.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define NUM_WRITERS 2
#define NUM_READERS 2
#define NUM_ITER 8192

int my_procnum = 0;
int pid;
int* pids;

void done(int sig) {
	int i;
	for(i = 0; i<NUM_READERS; i++) {
		kill(pids[i], SIGKILL);
	}
}

int main() {
	int parentPid = getpid();
	struct fifo* ff = mmap(NULL, sizeof(struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fifo_init(ff);
	printf("Spawning %d writers...\n", NUM_WRITERS);
	while(my_procnum < NUM_WRITERS) {
		pid = fork();
		if(pid == 0) {
			printf("Created writer %d with pid %d\n", my_procnum, getpid());
			int i;
			for(i=1 ; i<NUM_ITER; i++) {
				// printf("Writer %d writing %d (%d)\n", my_procnum, NUM_ITER*my_procnum+i, i);
				fifo_wr(ff, NUM_ITER*my_procnum + i);
			}
			printf("Writer %d wrote %d values.\n", my_procnum, NUM_ITER-1);
			exit(0);
		}
		my_procnum++;
	}

	char* mutex = mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*mutex = 0;
	int* history = mmap(NULL, sizeof(int)*NUM_WRITERS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	int* wrongCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	pids = mmap(NULL, sizeof(int)*(NUM_READERS), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	printf("Spawning %d readers...\n", NUM_READERS);
	while(my_procnum < NUM_WRITERS+NUM_READERS) {
		pid = fork();
		if(pid == 0) {
			printf("Created reader %d with pid %d\n", my_procnum, getpid());
			unsigned long cur, last = 0;
			int readerWrongCount = 0;
			while(1) {
				while(tas(mutex));
				cur = fifo_rd(ff);
				int wr_procnum = cur/NUM_ITER;
				unsigned long val = cur%NUM_ITER;
				// printf("Reader %d read %lu (%lu from %d)\n", my_procnum, cur, val, wr_procnum);
				if(val-history[wr_procnum] != 1) {
					printf("Incorrect value for writer %d: %lu, expected %d!\n", wr_procnum, val, history[wr_procnum]+1);
					readerWrongCount++;
					*wrongCount++;
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
				*mutex = 0;
				if(breakCount == NUM_WRITERS) {
					break;
				}
			}
			printf("Reader %d finished. %d incorrect, last read %lu.\n",my_procnum, readerWrongCount, last);
			kill(parentPid, SIGUSR2);
			exit(0);
		} else {
			pids[my_procnum-NUM_READERS] = pid;
		}
		my_procnum++;
	}
	signal(SIGUSR2, done);
	while(wait(NULL) != -1);
	printf("Done. %d total incorrect.\n", *wrongCount);


	return 0;
}
