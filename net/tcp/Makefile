cc=g++

.PHONY:all
all: tcpclient tcpserver

tcpclient:tcpclient.cc
	$(cc) -o $@ $^ -std=c++11
tcpserver:tcpserver.cc
	$(cc) -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -fr tcpclient tcpserver