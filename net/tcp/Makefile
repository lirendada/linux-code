cc=g++
.PHONY:all
all:tcpserver tcpclient

tcpclient:tcpclient.cc tcpclient.hpp
	$(cc) -o $@ $^ -std=c++11

tcpserver:tcpserver.cc tcpserver.hpp
	$(cc) -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -f tcpserver tcpclient *.txt