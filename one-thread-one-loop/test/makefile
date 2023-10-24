.PHONY:clean client server
all: client server

client : tcp_client.cc
	g++ -o $@ $^ -std=c++11
server : tcp_server.cc
	g++ -g -o $@ $^ -std=c++11

clean:
	rm -rf client server
