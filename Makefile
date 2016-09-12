CC = g++ -std=c++11

all: matrix test

matrix: matrix.cc
		$(CC) -o matrix matrix.cc -pthread

test: test.cc
		$(CC) -o test test.cc -pthread

clean:
	rm -rf matrix test
