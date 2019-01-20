bin=test
bin1=test1
cc=g++

all:$(bin1)

.PHONY:$(bin) $(bin1)
$(bin):TestUnit.cpp
	$(cc) -g -std=c++11 -o $@ $^
$(bin1):test.cpp
	$(cc) -g -std=c++11 -o $@ $^

.PHONY:clean
clean:
	rm -rf test test1 core*
