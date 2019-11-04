# makefile for myftp

myftp: myftp.o token.o
	gcc myftp.o token.o -o myftp

myftp.o: myftp.c token.h
	gcc -c myftp.c

token.o: token.c token.h
	gcc -c token.c

clean: 
	rm *.o
