CC = g++ -std=c++11

all: matrix test example

matrix: matrix.cc
		$(CC) -o matrix matrix.cc -pthread

test: test.cc
		$(CC) -o test test.cc -pthread

example: examples/example.cc
	$(CC) -o examples/example examples/example.cc -pthread

clean:
	rm -rf matrix test examples/example
