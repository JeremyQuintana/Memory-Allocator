default: all

canCompile: run clean

all: run

clean:
	rm -f run *.o

run: Allocator.o Chunk.o Experiment.o MemoryAllocator.o Locker.o
	g++ -Wall -Werror -std=c++14 -O -o $@ $^ -lpthread

%.o: %.cpp
	g++ -Wall -Werror -std=c++14 -O -c $^
