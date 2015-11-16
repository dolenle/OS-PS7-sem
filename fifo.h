/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.3 FIFO
	Prof. Hakner
*/

#include "sem.h"

#define MYFIFO_BUFSIZ 4096

struct fifo {
	unsigned long buf[MYFIFO_BUFSIZ];
	int read_addr, write_addr;	
	struct sem mutex, read_sem, write_sem;
};

void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f, unsigned long d);
unsigned long fifo_rd(struct fifo *f);