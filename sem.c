/*
	ECE357 Operating Systems
	Dolen Le
	PS 7.2 Semaphore
	Prof. Hakner
*/

#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void sigusr_wake(int signum) {
	return;
}

const struct sigaction sigusr_act = {
	.sa_handler = sigusr_wake,
	.sa_mask = 0,
	.sa_flags = 0
};

void sem_init(struct sem *s, int count) {
	s->count = count;
	s->lock = 0;
}

int sem_try(struct sem *s) {
	sigset_t all, oldset;
	sigfillset(&all);
	sigprocmask(SIG_BLOCK, &all, &oldset); //block signals in critical region

	while(tas(&(s->lock))); //spin
	int retval = 0;
	if(s->count > 0) {
		s->count--;
		retval = 1;
	}
	s->lock = 0;
	sigprocmask(SIG_SETMASK, &oldset, NULL);
	return retval;
}

void sem_wait(struct sem *s) {
	// printf("sem_wait %d\n", my_procnum);
	sigset_t all, usr_only, oldset;
	struct sigaction oldaction;
	sigfillset(&all);
	sigfillset(&usr_only);
	sigdelset(&usr_only, SIGUSR1);
	sigdelset(&usr_only, SIGINT);
	sigprocmask(SIG_BLOCK, &all, &oldset); //block signals in critical region
	while(tas(&(s->lock))); //wait for lock
	
	for(;;) {
		if(s->count > 0) {
			s->count--;
			s->cpus[my_procnum] = 0;
			s->lock = 0;
			break;
		} else { //block
			sigaction(SIGUSR1, &sigusr_act, &oldaction); //intercept SIGUSR1
			s->cpus[my_procnum] = getpid();
			s->lock = 0;
			// printf("suspending %d\n",s->cpus[my_procnum]);
			sigsuspend(&usr_only); //returns when semaphore increments
			// printf("\t\twoken\n");
			// sigaction(SIGUSR1, &oldaction, NULL); //restore original SIGUSR1 action
		}
	}
	sigprocmask(SIG_SETMASK, &oldset, NULL); //restore original mask
}

void sem_inc(struct sem *s) {
	// printf("sem_inc %d\n", my_procnum);
	sigset_t all, oldset;
	sigfillset(&all);
	sigprocmask(SIG_BLOCK, &all, &oldset); //block signals in critical region
	while(tas(&(s->lock))); //spin
	s->count++;
	int i;
	for(i = 0; i < N_PROC; i++) {
		// printf("check cpu %d: %d\n", i, s->cpus[i]);
		if(s->cpus[i] != 0) {
			// printf("waking %d\n",s->cpus[i]);
			kill(s->cpus[i], SIGUSR1); //wake next cpu
			// s->cpus[i] = 0;
			break;
		}
	}
	s->lock = 0;
	sigprocmask(SIG_SETMASK, &oldset, NULL);
}

// int main() {
// 	return 0;
// }