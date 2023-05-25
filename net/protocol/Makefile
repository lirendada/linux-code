cc=g++
LD=#-DMYSELF
.PHONY:all
all:tcpserver tcpclient

tcpclient:tcpclient.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp ${LD}

tcpserver:tcpserver.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp ${LD}

.PHONY:clean
clean:
	rm -f tcpserver tcpclient *.txt