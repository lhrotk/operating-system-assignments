all: sigperf.out sigshooter.out
	@echo ==============Automatic Execute==================
	@./sigperf.out
.PHONY: all

sigperf.out: sigperf.o
	cc -o sigperf.out sigperf.o
	@echo linking sigperf.o

sigshooter.out: sigshooter.o
	cc -o sigshooter.out sigshooter.o

sigshooter.o: sigshooter.c
	cc -c sigshooter.c -w -o sigshooter.o
 
sigperf.o: sigperf.c
	cc -c sigperf.c -o sigperf.o

.PHONY: clean man
clean: 
	-rm *.out *.o

man:	
	@cat ./manv1

