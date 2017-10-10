/*
   Author: Hongru Li 215245830

   I suggest you to set lambda and mu bigger or the result will be too random
   with 1000 ticks.
   
   Explanation:

   This program did not use pthread_cond_wait and pthread_cond_signal because
   the clock and the set of servers and clients wait each other like the last 
   assignment and is complicated. However, the data is well protected by a
   mutex and no server or client can be emited more than once in a signal tick.

   Also this method is busy waiting, it will not cause dead lock caused when 
   pthread_cond_wait is not ready but the pthread_cond_signal is excuted. To
   avoid deadlock in pthread_cond_wait also need busy waiting unless the some
   atomic operation is provided.

   In this assignment, the clock should emit next tick when everything is done.
   But if the last server or client tells the clock when it is done, a paradox 
   generated. If you are able to tell the clock, you are not suspend and ready
   for the next tick.

   but if a tick is one second or half a second or somthing, the clock and users
   can use pthread_cond_wait and pthread_cond_signal.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#define		BUSY	0
#define		FREE	1
#define		ROUNDS	1000
#define		WRONG_MESSAGE	"Wrong syntax of commandline, please refer to the manual in makefile to correct it!\n"
struct	quene{
	int		tasks;
	int 		times;
	int		generated;
	pthread_mutex_t	mutex;
	int		number_of_client;
	int		number_of_wait_client;	//also a signal in method signal() and wait();
	int 		number_of_server;
	int		number_of_wait_server;
	int		wait_time_total;
	int		total_execute_time;
	int		total_quene_length;	
	double		lambda;
	double		mu;
}quene;

void* clocker(void* arg){
	struct quene* quene;
	int	i;
	quene = (struct quene*) arg;
	quene->wait_time_total = 0;
	quene->total_quene_length = 0;
	quene->total_execute_time = 0;
	for(i=0; i < ROUNDS; i++){
		while(1){
			if(quene->number_of_wait_server == 0 && quene->number_of_wait_client == 0){
				break;
			}
		}	//wait(S), S is the signal, S = number_of_wait_client + number_of_wait_client
			if(pthread_mutex_lock(&quene->mutex) != 0){
			pthread_exit((void*) 1);
		}
		
		quene->number_of_wait_server = quene->number_of_server; //equals to wait() many times
		quene->number_of_wait_client = quene->number_of_client;
		quene->times++;
		quene->wait_time_total += quene->tasks;
	       	quene->total_quene_length += quene->tasks;

		printf("In tick %d, number of remaining task: %d\n", i, quene->tasks);

		if(pthread_mutex_unlock(&quene->mutex) != 0){
			pthread_exit((void*) 2);
		}
	}
	//printf("%d,%d", quene->wait_time_total, quene->generated);
	printf("AWT: %lf ticks\n", ((double)quene->wait_time_total)/quene->generated);
	printf("AXT: %lf ticks\n", ((double)quene->total_execute_time)/quene->generated);
	printf("ATA: %lf ticks\n", ((double)quene->wait_time_total)/quene->generated + ((double)quene->total_execute_time)/quene->generated);
	printf("AQL: %lf tasks\n", ((double)quene->total_quene_length)/quene->times);
	printf("AIA: %lf ticks\n", ((double)quene->times)/quene->generated);
	pthread_exit((void*) 0);
}

void* client_thread(void* arg){
	struct quene* quene;
	int 	i;
	quene = (struct quene*) arg;
	for(i = 0; i< ROUNDS; i++){
		while(1){
			if(i + 1 == quene->times && quene->number_of_wait_client != 0)
			/* 
			this condition is important, check the loop register to ensure
			one client will not get the mutex twice at one tick.
			*/
				break;
		}
		if(pthread_mutex_lock(&quene->mutex) != 0){
			pthread_exit((void*) 1);
		}

		if(rand()/(RAND_MAX + 1.00) < quene->lambda){
			quene->tasks++;
			quene->generated++;
		}

		quene->number_of_wait_client--;	//equals to signal(S)

		if(pthread_mutex_unlock(&quene->mutex) != 0){
			pthread_exit((void*) 2);
		}
	}
	pthread_exit((void*) 0);
}

void* server_thread(void* arg){
	struct quene* quene;
	int 	i;
	int	state = FREE;
	double	k;
	quene = (struct quene*) arg;
	for(i = 0; i< ROUNDS; i++){
		while(1){
			if(i + 1 == quene->times && quene->number_of_wait_client == 0 && quene->number_of_wait_server != 0)
			/*
			the first condition make sure one server wake up once in a tick

			the second condition and the third condition make sure the servers
			wake up after clients so that the can execute a new generated task.
			*/
				break;
		}
		if(pthread_mutex_lock(&quene->mutex) != 0){
			pthread_exit((void*) 1);
		}
		
		if(state == BUSY){
			if(k=rand()/(RAND_MAX + 1.00) < quene->mu)
				state = FREE;
		}

		if(state == FREE){
			if(quene->tasks > 0){
				quene->tasks--;
				state = BUSY;
			}
		}

		if(state == BUSY){
			quene->total_execute_time++;
		}

		quene->number_of_wait_server--;

		if(pthread_mutex_unlock(&quene->mutex) != 0){
			pthread_exit((void*) 2);
		}
	}
	pthread_exit((void*) 0);
}

int main(int argc, char* argv[]){
	int	number_of_client = 2;
	int	number_of_server = 2;
	int	number_of_arg;
	void*	error_number;
	pthread_t	client[5], server[5], clock_thread;
	int	i;
	quene.lambda = 0.005;
	quene.mu = 0.01;
	quene.tasks = 0;
	quene.times = 0;
	quene.generated = 0;
	if((argc-1)%2 != 0){
		printf(WRONG_MESSAGE);
		exit(0);
	}

	number_of_arg = (argc-1)/2;

	srand((unsigned) time(NULL));
	

	/* this loop reads the arguments*/

	for(i = 0; i < number_of_arg; i++){
		char 	buf[10];
		double 	temp;
		if(sscanf(argv[2*i + 1],"--%s", buf) == 0){
			printf(WRONG_MESSAGE);
			exit(0);
		}
		
		if(sscanf(argv[2*i + 2],"%lf", &temp) == 0){
			printf(WRONG_MESSAGE);
			exit(0);
		}
		//printf("%s, %lf", buf, temp);
		if(strcmp("lambda", buf) == 0){
			quene.lambda = temp;
			printf("lambda is %lf\n", quene.lambda);
		}
		else if(strcmp("mu", buf) == 0){
			quene.mu = temp;
			printf("mu is %lf\n", quene.mu);
		}
		else if(strcmp("servers", buf) == 0){
			number_of_server = temp;
			printf("number of servers is %d\n", number_of_server);
		}
		else if(strcmp("clients", buf) == 0){
			number_of_client = temp;
			printf("number of clients is %d\n", number_of_client);
		}
		else{
			printf(WRONG_MESSAGE);
			exit(0);
		}
	}

	/* initial the mutex and store the argument in quene*/

	quene.number_of_server = number_of_server;
	quene.number_of_client = number_of_client;
	quene.number_of_wait_client = 0;
	quene.number_of_wait_server = 0;

	if(pthread_mutex_init(& quene.mutex, NULL) != 0){
		fprintf(stderr, "%s: can not init mutex: %s\n", argv[0], strerror(errno));
		exit(0);
	}

	if(pthread_create(&clock_thread, NULL, clocker, &quene) != 0){
		fprintf(stderr, "%s: can not create clock thread: %s\n", argv[0], strerror(errno));
		exit(0);
	}

	for(i = 0; i < number_of_client; i++){
		if(pthread_create(&client[i], NULL, client_thread, &quene)!= 0){
			fprintf(stderr, "%s: can not create client thread: %s\n", argv[0], strerror(errno));
			exit(0);
		}
	}

	for(i = 0; i < number_of_server; i++){
		if(pthread_create(&server[i], NULL, server_thread, &quene)!= 0){
			fprintf(stderr, "%s: can not create client thread: %s\n", argv[0], strerror(errno));
			exit(0);
		}
	}

	if(pthread_join(clock_thread, &error_number) != 0){
		fprintf(stderr, "%s: pthread join error: %s", argv[0], strerror(errno));
		exit(0);
	}

	switch((long)error_number){
		case 1:
			fprintf(stderr ,"%s: can not lock mutex in clocker: %s\n", argv[0], strerror(errno));
			break;
		case 2:
			fprintf(stderr ,"%s: can not unlock mutex in clocker: %s\n", argv[0], strerror(errno));
			break;
	}
	exit(0);
} 
