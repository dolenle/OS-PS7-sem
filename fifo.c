/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.3 FIFO
	Prof. Hakner
*/

#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

void fifo_init(struct fifo *f) {
	f->read_addr = f->write_addr = 0;
	// f->write_sem = mmap(NULL, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
	// f->read_sem = mmap(NULL, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
	// f->mutex = mmap(NULL, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
	sem_init(&(f->write_sem), MYFIFO_BUFSIZ);
	sem_init(&(f->read_sem), 0);
	sem_init(&(f->mutex), 1);
}

void fifo_wr(struct fifo *f, unsigned long d) {
	// printf("fifo write\n");
	sem_wait(&(f->write_sem));
	sem_wait(&(f->mutex));
	f->buf[f->write_addr] = d;
	f->write_addr = (f->write_addr+1) % MYFIFO_BUFSIZ;
	sem_inc(&(f->mutex));
	sem_inc(&(f->read_sem)); //wake reader
	// printf("fifo write done\n");
}

unsigned long fifo_rd(struct fifo *f) {
	// printf("fifo read\n");
	sem_wait(&(f->read_sem));
	//sem_wait(&(f->mutex));
	unsigned long d = f->buf[f->read_addr];
	f->read_addr = (f->read_addr+1) % MYFIFO_BUFSIZ;
	//sem_inc(&(f->mutex));
	sem_inc(&(f->write_sem)); //wake writer
	return d;
}






