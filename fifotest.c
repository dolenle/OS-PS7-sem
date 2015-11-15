#include "fifo.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <setjmp.h>
#include <errno.h>

int my_procnum = 0;
unsigned long last, cur;

int main(){
	struct fifo *ff = mmap(NULL, sizeof(struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(ff == MAP_FAILED) {
		perror("mmap failed");
		exit(-1);
	}
	fifo_init(ff);
	
	if(fork() == 0) {
		printf("Writer PID %d\n", getpid());
		unsigned long i;
		for(i=1; i<2<<12; i++) {
			printf("Write %lu\n", i);
			fifo_wr(ff, i);
		}
		fifo_wr(ff, 0);
		// fifo_wr(ff, 0);
		printf("done writing\n");
		exit(0);
	} else {
		printf("Reader PID %d\n", getpid());
		my_procnum = 1;
		last = 0;
		while((cur = fifo_rd(ff)) != 0) {
			printf("Read %lu\n", cur);
			if(cur-last != 1) {
				printf("Incorrect value!\n");
				exit(-1);
			}
			last = cur;
		}
	}
	while(wait(NULL) != -1);
	printf("Done. Last read %lu\n", last);
	return 0;
}
