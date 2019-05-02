CXXFLAGS= -std=c++14 -Wall -Wextra -g
LDFLAGS=

rvcc: rvcc.o parse.o codegen.o
	${CXX} ${LDFLAGS} -o $@ $^

test: rvcc
	./test.sh

clean:
	rm -f rvcc *.o tmp*

