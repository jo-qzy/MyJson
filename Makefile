bin=test
cc=g++

all:$(bin)

.PHONY:$(bin)
$(bin):TestUnit.cpp
	$(cc) -g -std=c++11 -o $@ $^

.PHONY:clean
clean:
	rm test
