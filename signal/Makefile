.PHONY:all
all:mysignal mytest

mysignal:signal.cpp
	g++ -o $@ $^ -std=c++11 -g
mytest:test.cpp
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f mysignal