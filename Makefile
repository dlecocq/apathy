CPP = g++
CPPOPTS = -O3 -Wall -Werror -Werror=effc++ -g

all: test

driver: driver.cpp path.hpp
	$(CPP) $(COPPOPTS) -o driver driver.cpp

test: test.cpp path.hpp
	$(CPP) $(CPPOPTS) -o test test.cpp -isystem Catch/single_include
	./test

clean:
	rm -rdf test
