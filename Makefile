bin=json_test
cc=g++

all:$(bin) example

.PHONY:$(bin) example
$(bin):test.cpp
	$(cc) -g -std=c++11 -o $@ $^
example:example.cpp
	$(cc) -g -std=c++11 -o $@ $^

.PHONY:clean
clean:
	rm -rf $(bin) example core*
