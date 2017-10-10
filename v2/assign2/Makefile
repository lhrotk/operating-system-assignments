all: sercli.out
	@echo ==============To Execute==================
	@echo The executable file is sercli.out
	@echo The command line options to the program are the --lambda [0.005], --mu [0.01], --servers[2], --clients[2].
.PHONY: all

sercli.out:
	gcc -o sercli.out sercli.c  -lpthread

.PHONY: clean man
clean: 
	-rm *.out

man:	
	@less ./manpage

