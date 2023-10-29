.PHONY:clean client server
all: client server

client : tcp_client.cc
	g++ -o $@ $^ -std=c++11
server : server.cc
	g++ -g -o $@ $^ -std=c++11 -lpthread

clean:
	rm -rf client server
