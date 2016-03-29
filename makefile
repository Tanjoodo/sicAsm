CXX = g++
override CXXFLAGS += -Wall -std=c++11

debug: CXXFLAGS += -g
debug: all

all: main.o int24.o
	$(CXX) $(CXXFLAGS) main.o int24.o -o sicAsm

main.o: int24.o
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o main.o

int24.o:
	$(CXX) $(CXXFLAGS) -c src/int24.cpp -o int24.o

clean:
	rm *.o sicAsm
