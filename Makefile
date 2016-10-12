CC = g++ -std=c++11 -O3
# CC = g++ -std=c++11 -O3 -ggdb
#CC = g++ -std=c++11 -O0 -ggdb

all: matrix test example dataset compressed

matrix: matrix.cc thread_pool
		$(CC) -o matrix matrix.cc -pthread

test: test.cc
		$(CC) -o test test.cc -pthread

thread_pool: thread_pool.cc
	$(CC) -o thread_pool thread_pool.cc -pthread

dataset: LoadDataset.cc lib/SparseMatrix.hpp
	$(CC) -o dataset LoadDataset.cc -pthread -DARMA_DONT_USE_WRAPPER -lopenblas -llapack

example: examples/example.cc
	$(CC) -o examples/example examples/example.cc -pthread

compressed: compressed.cc
		$(CC) -o comp compressed.cc -pthread -DARMA_DONT_USE_WRAPPER -lopenblas -llapack

clean:
	rm -rf matrix test examples/example thread_pool dataset tp
