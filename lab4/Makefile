all: primecount sort main_compile main_dl link

link: 
	gcc primecount.o sort.o main_compile.o -o app_compile.out
	gcc main_dl.o -o app_dl.out

main_compile: main_compile.c
	gcc -c main_compile.c -o main_compile.o

main_dl: main_dl.c
	gcc -c main_dl.c -o main_dl.o

primecount: primecount.c
	gcc -c primecount.c -o primecount.o
	gcc -shared -rdynamic primecount.c -o primecount.so

sort: sort.c
	gcc -c sort.c -o sort.o
	gcc -shared -rdynamic sort.c -o sort.so

clean:
	rm *.out
	rm *.so
	rm *.o