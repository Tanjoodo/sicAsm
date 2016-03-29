CC = g++
override CFLAGS += -Wall -std=c++11

all: main.o int24.o
	$(CC) $(CFLAGS) main.o int24.o -o sicAsm

main.o: int24.o
	$(CC) $(CFLAGS) -c src/main.cpp -o main.o

int24.o:
	$(CC) $(CFLAGS) -c src/int24.cpp -o int24.o

clean:
	rm *.o sicAsm
