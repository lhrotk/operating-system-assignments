#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char* argv[]){
	pid_t		child_1, child_2, temp;
	char    	buf[20];
	sigset_t	newmask;

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR2);
	sigaddset(&newmask, SIGUSR1);

	if(sigprocmask(SIG_BLOCK, &newmask, NULL) <0){
		fprintf(stderr, "%s: block signal error: %s\n", argv[0], strerror(errno));
		exit(1);
	}

	if((child_1 = fork()) < 0){
		fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
		exit(1);
	}
	else if(child_1 == 0){
		execlp("./sigshooter.out", "./sigshooter.out", NULL);
	}

	if(sprintf(buf, "%d", child_1) == -1){
		fprintf(stderr, "%s: sprintf error: %s\n",argv[0] , strerror(errno));
		exit(0);
	}
        //printf("%s\n", buf);


	if((child_2 = fork()) < 0){
		fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
	}
	else if (child_2 == 0){
		execlp("./sigshooter.out", "./sigshooter.out", buf , NULL);
	}
        
	printf("child1 id: %d, child2 id: %d\n", child_1, child_2);
        
        temp = wait(NULL);
	if(child_1 == temp){
		printf("done1\n");
	}
	else if(temp == child_2){
		printf("done2\n");
	}
	temp = wait(NULL);
	if(child_1 == temp){
		printf("done1\n");
	}
	else if(temp == child_2){
		printf("done2\n");
	}
	exit(0);
}
