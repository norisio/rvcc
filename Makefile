CXXFLAGS= -std=c++14 -Wall -Wextra
rvcc: rvcc.cpp

test: rvcc
	./test.sh

clean:
	rm -f rvcc *.o tmp*

