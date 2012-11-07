CPP = g++
CPPOPTS = -O3 -Wall -Werror -g

all: test

test: test.cpp path.hpp
	$(CPP) $(CPPOPTS) -o test test.cpp -ICatch/single_include
	./test

clean:
	rm -rdf test
