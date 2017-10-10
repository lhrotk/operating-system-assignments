#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#define  readpipe "1.pipe"
#define  sendpipe "2.pipe"
int main(int argc, char* argv[]){
	pid_t		child_1, child_2, temp;
	sigset_t	newmask;
	char    	sendbuf[20], readbuf[20];
	int 		fd[2];

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);

	if(sigprocmask(SIG_BLOCK, &newmask, NULL) <0){
		fprintf(stderr, "%s: block signal error: %s\n", argv[0], strerror(errno));
		exit(1);
	}

 	if((mkfifo(readpipe, 0777)) < 0){
		fprintf(stderr, "%s:open readpipe error: %s\n", argv[0], strerror(errno));
		exit(1);
	}	

	if(mkfifo(sendpipe, 0777) < 0){
		fprintf(stderr ,"%s: open sendpipe error: %s\n", argv[0], strerror(errno));
		remove(readpipe);
		exit(1);
	}

	if((child_1 = fork()) < 0){
		fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
		exit(1);
	}
	else if(child_1 == 0){
		temp = getpid();
		sprintf(sendbuf ,"%d", temp);
		//printf("child1 :%s\n", sendbuf);/*you MUST add \n because it flush IO, or the IO will disappear for execlp!!!*/
		if((fd[0] = open(sendpipe, O_WRONLY)) < 0){
			fprintf(stderr, "%schild1 open readpipe error: %s\n", argv[0], strerror(errno));
			exit(1);
		}
		if((fd[1] = open(readpipe, O_RDONLY)) < 0){
			fprintf(stderr, "%s: child1 open sendpipe error: %s\n", argv[0], strerror(errno));
			exit(1);
		}
		write(fd[0], sendbuf, sizeof(sendbuf));
		read(fd[1], readbuf, sizeof(readbuf));
		close(fd[0]);
		close(fd[1]);
		//printf("The PID of child2 is: %s\n", readbuf);
		execlp("./sigshooter.out", "./sigshooter.out", readbuf, "1", NULL);
	}

	if((child_2 = fork()) < 0){
		fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
		exit(1);
	}
	else if (child_2 == 0){
		temp = getpid();
		sprintf(sendbuf, "%d", temp);
		if((fd[0] = open(sendpipe, O_RDONLY)) < 0){
			fprintf(stderr, "%s: child2 open readpipe error:%s\n", argv[0], strerror(errno));
			exit(1);
		}

		if((fd[1] = open(readpipe, O_WRONLY)) < 0){
			fprintf(stderr ,"%s: child2 open sendpipe error: %s", argv[0], strerror(errno));
			exit(1);
		}
		read(fd[0], readbuf, sizeof(readbuf));
		write(fd[1], sendbuf, sizeof(sendbuf));
		//printf("The pid of child1 is: %s\n", readbuf);
		close(fd[0]);
		close(fd[1]);
		execlp("./sigshooter.out","./sigshooter.out", readbuf, "2", NULL);
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

	remove(readpipe);
	remove(sendpipe);

	exit(0);
}
