CC = g++ -std=c++11 -O3

all: matrix test example

matrix: matrix.cc thread_pool
		$(CC) -o matrix matrix.cc -pthread

test: test.cc
		$(CC) -o test test.cc -pthread

thread_pool: thread_pool.cc
	$(CC) -o thread_pool thread_pool.cc -pthread

example: examples/example.cc
	$(CC) -o examples/example examples/example.cc -pthread

clean:
	rm -rf matrix test examples/example thread_pool
