#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dlfcn.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<unistd.h>
#include<fcntl.h>

int my_cnt(char* filename){
	int	fd;
	struct  stat	buf;
	char*	check;
	int	i, sum;
	if((fd = open(filename, O_RDONLY)) < 0 ){
		fprintf(stderr, "can't open the file\n");
		return -1;
	}

	if((fstat(fd, &buf)) < 0){
		fprintf(stderr, "fstat error\n");
		return -1;
	}//check the size of file

	check = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	//load the file to the memory (user space)
	close(fd);

	if(check == NULL){
		fprintf(stderr, "mmap error\n");
		return -1;
	}

	for(i = 0, sum = 0; i < (int)buf.st_size; i++){
		if(*(check + i) == '\n'){
			sum++;
		}
	}//check 

	munmap(check, buf.st_size);
	//cancel the map
	return sum;
}
