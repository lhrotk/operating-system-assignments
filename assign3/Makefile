all: nlcnt

.PHONY: all

nlcnt: nlcnt.c mapping.so reading.so
	gcc -rdynamic -o nlcnt nlcnt.c -ldl

mapping.so:
	gcc -fPIC -shared mapping.c -o mapping.so

reading.so:
	gcc -fPIC -shared reading.c -o reading.so

.PHONY: clean man
clean: 
	-rm *.out *.o nlcnt

man:	
	@cat ./manv
