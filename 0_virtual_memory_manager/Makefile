all: a.out
	./a.out 65536 0
	./a.out 65536 1
	./a.out 65536 2
	./a.out 65536 3
	./a.out 32768 0
	./a.out 32768 1
	./a.out 32768 2
	./a.out 32768 3
	./a.out 16384 0
	./a.out 16384 1
	./a.out 16384 2
	./a.out 16384 3

a.out: main.cpp pageTable.o
	g++ -std=c++11 main.cpp tlb.o pageTable.o env.o


tlb.o: tlb.h tlb.cpp
	g++ -c -std=c++11 tlb.cpp


pageTable.o: pageTable.h pageTable.cpp tlb.o
	g++ -c -std=c++11 pageTable.cpp

env.o: env.cpp env.h
	g++ -c env.cpp
