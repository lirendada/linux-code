all: client server

client : tcp_client.cc
	g++ -o $@ $^ -std=c++11
server : tcp_server.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -rf client server
