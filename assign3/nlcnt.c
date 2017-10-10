/****************************************
Author: Hongru Li
Student ID: 215245830

***************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dlfcn.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>


int main (int argc, char* argv[]){
	int	i, result;
	void*	handle;
	char*	error;
	char	filename[20] = "./";
	int	(*my_cnt)(char* filename);
	if(argc != 3){
		fprintf(stderr, "wrong argument!\n");
		exit(0);
	}
	else{
		strcat(filename, argv[1]);
		strcat(filename, ".so");
	}//get the directory for dynamic library
		
	handle = dlopen(filename, RTLD_LAZY);
	//connect to *.so
	if(!handle){
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	dlerror();

	*(void**) (&my_cnt) = dlsym(handle, "my_cnt");//copy the function from *.so

	if((error = dlerror()) != NULL){
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		exit(EXIT_FAILURE);
	}

	if((result = (*my_cnt)(argv[2])) < 0){
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		exit(0);
	}

	printf("number of new-line character: %d\n", result);
	exit(0);


		
}
