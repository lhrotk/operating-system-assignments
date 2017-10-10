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
	char	check;
	int	sum = 0;
	if((fd = open(filename, O_RDONLY)) < 0 ){
		fprintf(stderr, "can't open the file\n");
		return -1;
	}//open the file

        while(read(fd, &check, 1) != 0){
		if(check == '\n'){
			sum ++; 
		}
        }//read one chatacter at a time and check

	close(fd);


	return sum;
}

