cc=g++
.PHONY:all
all:tcpserver tcpclient

tcpclient:tcpclient.cc
	$(cc) -o $@ $^ -std=c++11

tcpserver:tcpserver.cc
	$(cc) -o $@ $^ -std=c++11 #-lpthread

.PHONY:clean
clean:
	rm -f tcpserver tcpclient *.txt