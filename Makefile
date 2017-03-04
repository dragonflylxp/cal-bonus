NPO : main.o pub.o heapsort.o bitmap.o
	gcc -o NPO bitmap.o heapsort.o main.o pub.o 
bitmap.o : bitmap.c bitmap.h
	gcc -Wall -g  -c bitmap.c 
heapsort.o : heapsort.c heapsort.h
	gcc -Wall -g  -c heapsort.c 
pub.o : pub.c pub.h
	gcc -Wall -g  -c pub.c 
main.o : main.c
	gcc -Wall -g  -c main.c 

clean :
	rm NPO bitmap.o heapsort.o pub.o main.o
