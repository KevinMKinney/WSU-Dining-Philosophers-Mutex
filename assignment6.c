#include <unistd.h>
//#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "random.h"

// -- MACROS FOR CONFIGURATION --
// for submission: 5, 100, 1, 9, 3, 1, 11, 7
#define philosophers 5
#define totalEatingTime 12
#define minimumEatingTime 1
#define eatingMean 9
#define eatingSD 3
#define minimumThinkingTime 1
#define thinkingMean 11
#define thinkingSD 7

pid_t wait(int *wstatus);

/* successive calls to randomGaussian produce integer return values */
/* having a gaussian distribution with the given mean and standard  */
/* deviation.  Return values may be negative.                       */

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}

int macroCheck() {
	// only checking macro value
	if ((philosophers <= 0) || 
		(totalEatingTime <= 0) || 
		(minimumEatingTime <= 0) || 
		(minimumThinkingTime <= 0)){
		return -1;
	} else {
		return 1;
	}
}

// global array for threads
pthread_mutex_t chopsticks[philosophers];

void* threadPhilosopher(void* thrNum) {

	// init vars
	int i = *(int*)thrNum;
	int eatTime, thinkTime, R, totalTime = 0;

	// random seed
	time_t t;
	srand(time(&t)+i);

	if ((i+1) >= philosophers) {
		R = 0;
	} else {
		R = i+1;
	}

	while (totalTime <= totalEatingTime) {

		// think logic
		thinkTime = randomGaussian(thinkingMean, thinkingSD);
		if (thinkTime < minimumThinkingTime) {
			thinkTime = minimumThinkingTime;
		}
		printf("Philosopher %d has started thinking\n", (i+1));
		sleep(thinkTime);
		printf("Philosopher %d thought for %d seconds\n", (i+1), thinkTime);

		// try to pick up left chopstick, then the right.
		// if it cannot pick up the right chopstick, put down the left.
		for (;;) {
			if (pthread_mutex_lock(&chopsticks[i]) != 0) {
				fprintf(stderr, "%s\n", strerror(errno));
				exit(0);
			}
			if (pthread_mutex_trylock(&chopsticks[R]) == EBUSY) {
				if (pthread_mutex_unlock(&chopsticks[i]) != 0) {
					fprintf(stderr, "%s\n", strerror(errno));
					exit(0);
				}	
			} else {
				if (pthread_mutex_trylock(&chopsticks[R]) == -1) {
					fprintf(stderr, "%s\n", strerror(errno));
					exit(0);
				}
				break;
			}
		}

		// eat logic
		eatTime = randomGaussian(eatingMean, eatingSD);
		if (eatTime < minimumEatingTime) {
			eatTime = minimumEatingTime;
		}		
		printf("Philosopher %d has started eating\n", (i+1));
		sleep(eatTime);
		totalTime += eatTime;
		printf("Philosopher %d ate for %d seconds\n", (i+1), eatTime);
		
		// put down chopsticks
		if (pthread_mutex_unlock(&chopsticks[i]) != 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(0);
		}
		if (pthread_mutex_unlock(&chopsticks[R]) != 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(0);
		}
	}

	printf("Philosopher %d has finished eating\n", (i+1));
	free(thrNum);
	return NULL;
}

int main(){

	if (macroCheck() == -1) {
		printf("Macros are set incorreclty\n");
		return 0;
	}

	if (pthread_mutex_init(chopsticks, NULL) != 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(0);
	}

	pthread_t thr[philosophers];
	int* thrNum;

	// start threads
	for (int i = 0; i < philosophers; i++) {
		// should probably error check (it would probably break with a LOT of philosophers)
		thrNum = malloc(sizeof(int)); 
		*thrNum = i;
		if (pthread_create(&thr[i], NULL, threadPhilosopher, (void*)thrNum) != 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(0);
		}
	}

	// wait for threads to finish
	for (int i = 0; i < philosophers; i++) {
		if (pthread_join(thr[i], NULL) != 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(0);	
		}
	}

	// clean up mutex
	if (pthread_mutex_destroy(chopsticks) != 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(0);
	}

	printf("Philosophers are all done!\n");

	return 0;
}