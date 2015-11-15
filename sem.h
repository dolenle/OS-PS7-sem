/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.2 Semaphore
	Prof. Hakner
*/

#define N_PROC 4 //maximum allowed processors

extern int my_procnum; //make this extern later

struct sem {
	int count;
	volatile char lock;
	unsigned int cpus[N_PROC];
};

int tas(volatile char *spinlock);

void sem_init(struct sem *s, int count);
int sem_try(struct sem *s); //proberen te verlagen zonder blokkering (geef fout)
void sem_wait(struct sem *s); //proberen te verlagen anders blok
void sem_inc(struct sem *s); //verhogen seinpaal