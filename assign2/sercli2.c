/*
   Author: Hongru Li 215245830  version2

   I suggest you to set lambda and mu bigger or the result will be too random
   with 1000 ticks.
   
   Explanation:

   This is another solution. Using pthread_cond_broadcast and pthread_cond_wait
   But the number of the loop is different condition. To avoid one server runs twice
   at one tick, it should wait for the condition of the next tick. There is only 
   3 condition variable to use. (the condition of tick 1 comes is different of
   the condition tick 2 comes)
 
   After use the cond, initial it again so that we can use it again later.
   3 cond completely elimicate comflict.

   And, it is much much quicker than the other solution because there is no 
   busy waiting for servers and clients. Only the clock check a condition all the time.

   I came up this method at the last hour before the deadline, I do not know if 
   there is some existing problem.
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
	int		number_of_wait_client;	//the clock check it for next tick
	int 		number_of_server;
	int		number_of_wait_server;
	int		wait_time_total;	//data used to calculate ATA, AXT...
	int		total_execute_time;
	int		total_quene_length;	
	double		lambda;
	double		mu;
}quene;

pthread_mutex_t	mtx[3] = {PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t	cond[3] = {PTHREAD_COND_INITIALIZER};

//clock thread

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
			//check if everyone is done and set the signal for next tick
				if(pthread_mutex_lock(&quene->mutex) != 0){
					pthread_exit((void*) 1);
				}	
				
				quene->number_of_wait_server = quene->number_of_server;
				quene->number_of_wait_client = quene->number_of_client;
				//rewrite the signal to set N position available in the new tick.
				quene->times++;
				quene->wait_time_total += quene->tasks;
			       	quene->total_quene_length += quene->tasks;
				//record relevant data
		
				printf("In tick %d, number of remaining task: %d\n", i, quene->tasks);
				//tells you situation about the quene each tick.
	
				if(pthread_mutex_unlock(&quene->mutex) != 0){
					pthread_exit((void*) 2);
				}
				//wake up all threads
				pthread_mutex_lock(&mtx[i%3]);
				pthread_cond_broadcast(&cond[i%3]); 
				pthread_mutex_unlock(&mtx[i%3]);
				pthread_cond_init(&cond[(i-1)%3], NULL);  //init it to use again.
				break;
			}
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


//client thread
void* client_thread(void* arg){
	struct quene* quene;
	int 	i;
	quene = (struct quene*) arg;
	for(i = 0; i< ROUNDS; i++){
		pthread_mutex_lock(&mtx[i%3]);
		while(i + 1 != quene->times){
			
			/* 
			this condition is important, check the loop register to ensure
			one client will not get the mutex twice at one tick.
			*/
			pthread_cond_wait(&cond[i%3], &mtx[i%3]);
		}
		pthread_mutex_unlock(&mtx[i%3]);
		//all threads wake up together but only one can write the quene
		if(pthread_mutex_lock(&quene->mutex) != 0){
			fprintf(stderr ,"client: can not lock mutex in clocker: %s\n", strerror(errno));
			pthread_exit((void*) 0);
		}
		//generate tasks
		if(rand()/(RAND_MAX + 1.00) < quene->lambda){
			quene->tasks++;
			quene->generated++;
		}

		quene->number_of_wait_client--;
		
		if(pthread_mutex_unlock(&quene->mutex) != 0){
			fprintf(stderr ,"client: can not unlock mutex in clocker: %s\n", strerror(errno));
			pthread_exit((void*) 0);
		}
		//let others in
	}
	pthread_exit((void*) 0);
}

//server thread

void* server_thread(void* arg){
	struct quene* quene;
	int 	i;
	int	state = FREE;
	double	k;
	quene = (struct quene*) arg;
	for(i = 0; i< ROUNDS; i++){
		pthread_mutex_lock(&mtx[i%3]);
		while(i + 1 != quene->times){
			
			/* 
			this condition is important, check the loop register to ensure
			one client will not get the mutex twice at one tick.
			*/
			pthread_cond_wait(&cond[i%3], &mtx[i%3]);
		}
		pthread_mutex_unlock(&mtx[i%3]);

		//all servers wake up.
		if(pthread_mutex_lock(&quene->mutex) != 0){
			fprintf(stderr ,"server: can not lock mutex in clocker: %s\n", strerror(errno));
			pthread_exit((void*) 0);
		}
		
		if(state == BUSY){
			if((k=rand()/(RAND_MAX + 1.00)) < quene->mu)
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
			fprintf(stderr ,"server: can not unlock mutex in clocker: %s\n", strerror(errno));
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

	/*
	  number_of_wait_client and number_of_wait_server is 0 so clients and
	  servers will block to be activated at first.
	*/

	if(pthread_mutex_init(& quene.mutex, NULL) != 0){
		fprintf(stderr, "%s: can not init mutex: %s\n", argv[0], strerror(errno));
		exit(0);
	}

	//initial the mutex

	if(pthread_create(&clock_thread, NULL, clocker, &quene) != 0){
		fprintf(stderr, "%s: can not create clock thread: %s\n", argv[0], strerror(errno));
		exit(0);
	}

	//create the clock thread

	for(i = 0; i < number_of_client; i++){
		if(pthread_create(&client[i], NULL, client_thread, &quene)!= 0){
			fprintf(stderr, "%s: can not create client thread: %s\n", argv[0], strerror(errno));
			exit(0);
		}
	}

	//create the clients

	for(i = 0; i < number_of_server; i++){
		if(pthread_create(&server[i], NULL, server_thread, &quene)!= 0){
			fprintf(stderr, "%s: can not create client thread: %s\n", argv[0], strerror(errno));
			exit(0);
		}
	}

	//create the servers
	//if some error happens the main thread stop and all threads are killed

	if(pthread_join(clock_thread, &error_number) != 0){
		fprintf(stderr, "%s: pthread join error: %s", argv[0], strerror(errno));
		exit(0);
	}

	//only wait for the clock threads because all threads created will finish 
	//together.

	switch((long)error_number){
		case 1:
			fprintf(stderr ,"%s: can not lock mutex in clocker: %s\n", argv[0], strerror(errno));
			break;
		case 2:
			fprintf(stderr ,"%s: can not unlock mutex in clocker: %s\n", argv[0], strerror(errno));
			break;
	}
	//we can check wether clock thread ended correctly by checking the exit
	//code.
	exit(0);
} 
