CC = g++ -std=c++11 -O3
# CC = g++ -std=c++11 -O3 -ggdb
#CC = g++ -std=c++11 -O0 -ggdb

all: dataset test example

dataset: LoadDataset.cc lib/SparseMatrix.hpp
	$(CC) -o dataset LoadDataset.cc -pthread

example: examples/example.cc
	$(CC) -o examples/example examples/example.cc -pthread

test: test.cc
		$(CC) -o test test.cc -pthread

clean:
	rm -rf examples/example dataset sp
