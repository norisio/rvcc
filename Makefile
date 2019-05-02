CXXFLAGS= -std=c++14 -Wall -Wextra -g
LDFLAGS=
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

all: rvcc

$(OBJS): rvcc.hpp

rvcc: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

test: rvcc
	./test.sh

clean:
	rm -f rvcc *.o tmp*

